package com.quantatissu.orchestrator.controller.conference;

import com.quantatissu.orchestrator.form.VideoForm;
import com.quantatissu.orchestrator.model.conference.Room;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.ui.Model;
import org.springframework.validation.BindingResult;
import org.springframework.validation.annotation.Validated;
import org.springframework.web.HttpMediaTypeNotAcceptableException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.servlet.mvc.method.annotation.ResponseBodyEmitter;
import org.springframework.web.servlet.mvc.method.annotation.SseEmitter;

@RestController
public class SocketRestController {

    private SseEmitter emitter;
    private final ExecutorService cachedThreadPool = Executors.newCachedThreadPool();
    private static final String[] channels = {"channel1","channel2"};

    private static List<Room> rooms = new ArrayList<>();

    @RequestMapping(value = "/socket", method = RequestMethod.GET, produces=MediaType.TEXT_EVENT_STREAM_VALUE)
    @ResponseBody    
    public ResponseEntity<ResponseBodyEmitter> pollEvents() {
        emitter = new SseEmitter(180000L); // 3 minute timeout
        cachedThreadPool.execute(() -> {
            try {
                for (String channel : channels) {
                    emitter.send(SseEmitter.event()
                           .name("message").data("[{\"data\": \"" + channel + "\",\"response\":\"" + retrieveRoomList() + "\",\"channel\":\"" + channel + "\",\"event\":\"offer\"}]"));
                    TimeUnit.SECONDS.sleep(1);
                }
                emitter.complete();
            } catch (IOException | InterruptedException e) {
                emitter.completeWithError(e);
            }
        });

        return new ResponseEntity<>(emitter, createHeaders("poll-rooms"), HttpStatus.OK);
    }    
    
    @RequestMapping(value = "/socket/{id}", method = RequestMethod.GET, produces=MediaType.TEXT_EVENT_STREAM_VALUE)
    @ResponseBody    
    public ResponseEntity<ResponseBodyEmitter> connectRoom(@PathVariable("id") Integer id) {
        String responseText;
        String roomInfo;
        String roomCreated;
        List<Room> currentRooms = rooms.stream()
            .filter((room) -> Objects.equals(room.getId(), id))
            .collect(Collectors.toList());
        
        if (currentRooms.size() < 1){
            responseText = "No rooms with id "+id;
            roomInfo = "{\"roomInfo\":\"No room created\"}";
            roomCreated = "";
        } else {
            if (currentRooms.size() > 1){
                responseText = "Multiple rooms with id "+id;
                roomInfo = retrieveRoomInfo(id,rooms.get(0));
            } else {
                responseText = retrieveRoomSize(id,rooms.get(0));
                roomInfo = retrieveRoomInfo(id,rooms.get(0));
            }
            roomCreated = "\"RTCSessionDescription\":{\"type\":\""+rooms.get(0).getType()+"\",\"sdp\":\""+rooms.get(0).getSdp()+"\"},";
        }
        
        emitter = new SseEmitter(180000L);
        cachedThreadPool.execute(() -> {
            try {
                for (String channel : channels) {
                    emitter.send(SseEmitter.event()
                           .name("name").data("[{\"data\": \"" + channel + "\",\"response\":\"" + responseText + "\",\"room\":" + roomInfo + "," + roomCreated + "\"event\":\"answer\"}]"));
                    TimeUnit.SECONDS.sleep(1);
                }
                emitter.complete();
            } catch (IOException | InterruptedException e) {
                emitter.completeWithError(e);
            }
        });

        return new ResponseEntity<>(emitter, createHeaders("connect-room"), HttpStatus.OK);
    }    

    @PostMapping(path = "/socket/{id}", consumes = MediaType.APPLICATION_FORM_URLENCODED_VALUE)
    public String handleVideoStream(@Validated VideoForm file, BindingResult result){        
        return "redirect:/stream/media";
    }
    
    @RequestMapping(value = "/socket/{id}", method = RequestMethod.POST, produces=MediaType.TEXT_EVENT_STREAM_VALUE)  
    @ResponseBody 
    public ResponseEntity<ResponseBodyEmitter> postForEvents(
            @Validated VideoForm file, BindingResult result,
            @RequestParam String message, 
            @RequestParam String sdp, @RequestParam String type, @PathVariable("id") Integer id, Model model) {
        
        List<Room> currentRooms = rooms.stream()
            .filter((room) -> Objects.equals(room.getId(), id))
            .collect(Collectors.toList());
        
        if (currentRooms.size()>0){
            if (currentRooms.size()==1){
                currentRooms.get(0).setAttendees(currentRooms.get(0).getAttendees()+1);
            }
        } else {
            Room room = new Room(id, sdp, type, 1);
            rooms.add(room);
        }
        String roomInfo = retrieveRoomInfo(id,rooms.get(0));
        
        emitter = new SseEmitter(180000L);
        cachedThreadPool.execute(() -> {
            try {
                emitter.send(SseEmitter.event()
                       .name("message")
                       .data("[{\"data\":\"Post request for sockets"+message+"\","
                               + "\"room\":"+roomInfo+","
                               + "\"event\":\"answer\"}]"));
                emitter.complete();
            } catch (Exception e) {
                emitter.completeWithError(e);
            }
        });

        return new ResponseEntity<>(emitter, createHeaders("create-room"), HttpStatus.OK);
    }    

    @RequestMapping(value = "/socket/{id}/{command}", method = RequestMethod.POST, produces=MediaType.TEXT_EVENT_STREAM_VALUE)  
    @ResponseBody 
    public ResponseEntity<ResponseBodyEmitter> roomCommands(
            @RequestParam(required = false, defaultValue = "") String message, 
            @RequestParam(required = false, defaultValue = "") String sdp, 
            @RequestParam(required = false, defaultValue = "") String type, 
            @PathVariable("id") Integer id, 
            @PathVariable("command") String command, Model model) {
        
        final String responseText;
        
        if (null == command){
            responseText = "Received empty command ";
        } else switch (command) {
            case "close-room":
                List<Room> filteredRooms = rooms.stream()
                        .filter((room) -> !Objects.equals(room.getId(), id))
                        .collect(Collectors.toList());
                rooms = filteredRooms;
                responseText = "Room id "+id+" is closed.";
                break;
            case "close-all-rooms":
                rooms = new ArrayList<Room>();        
                responseText = "All rooms are closed.";
                break;
            default:
                responseText = "Received command " + command;
                break;
        }
        emitter = new SseEmitter(180000L);
        cachedThreadPool.execute(() -> {
            try {
                emitter.send(SseEmitter.event()
                       .name("message")
                       .data("[{\"data\":\""+responseText+"\","
                               + "\"event\":\"answer\"}]"));
                emitter.complete();
            } catch (Exception e) {
                emitter.completeWithError(e);
            }
        });

        return new ResponseEntity<>(emitter, createHeaders("room-commands"), HttpStatus.OK);
    }    

    @RequestMapping(value = { "/stream/media" }, method = RequestMethod.GET)
    public ResponseEntity<ResponseBodyEmitter> addStreamEmitter(@Validated VideoForm file, BindingResult result) {
        return new ResponseEntity<>(sendEmitterStream(), createHeaders("get-media-stream"), HttpStatus.OK);         
    }

    @RequestMapping(value = { "/stream/media" }, method = RequestMethod.POST)
    public ResponseEntity<ResponseBodyEmitter> addStreamMeda(@Validated VideoForm file, BindingResult result) {
        return new ResponseEntity<>(sendEmitterStream(), createHeaders("post-media-stream"), HttpStatus.OK);         
    }

    @RequestMapping(value = { "/stream/video" }, method = RequestMethod.GET)
    public ResponseEntity<ResponseBodyEmitter> addStreamVideoEmitter(@Validated VideoForm file, BindingResult result) {
        return new ResponseEntity<>(sendEmitterStream(), createHeaders("get-video-stream"), HttpStatus.OK);         
    }

    @RequestMapping(value = { "/stream/video" }, method = RequestMethod.POST)
    public ResponseEntity<ResponseBodyEmitter> addStreamVideo(@Validated VideoForm file, BindingResult result) {
        return new ResponseEntity<>(sendEmitterStream(), createHeaders("post-video-stream"), HttpStatus.OK);         
    }

    @ResponseBody
    @ExceptionHandler(HttpMediaTypeNotAcceptableException.class)
    public String handleHttpMediaTypeNotAcceptableException() {
        return "Media Type Exception:";
    }
    
    @GetMapping("/success")
    @ResponseBody
    public ResponseEntity<String> getSuccess() {
        return new ResponseEntity<String>("Success", HttpStatus.OK);
    }

    public static List<Room> getRooms() { return rooms; }
    public static void setRooms(List<Room> rooms) { SocketRestController.rooms = rooms; }

    private HttpHeaders createHeaders(String customHeader){
        HttpHeaders headers = new HttpHeaders();
        headers.add("Content-Type","text/event-stream");
        headers.add("Cache-Control","no-cache");
        headers.add("Custom-Event-Source",customHeader);
        return headers;
    }
    
    private String retrieveRoomList(){
        List<String> ids = new ArrayList<>(); 
        Iterator<Room> ri = rooms.iterator();
        while (ri.hasNext()) {
            Room r = ri.next();
            ids.add("room id "+r.getId());
        }
        return ids.toString().replaceAll(", ", " ").replaceAll("\\[|\\]", "");        
    }
    
    private String retrieveRoomInfo(Integer id, Room room) {
        return "[{\"sdp\":\""+room.getSdp()+"\"},"
                + "{\"attendees\": \""+room.getAttendees()+"\"},"
                + "{\"id\":\""+id+"\"},"
                + "{\"type\": \""+room.getType()+"\"}]";
    }

    private String retrieveRoomSize(Integer id, Room room) {
        return "1 room with id: "+id+" and attendees: "+room.getAttendees();
    }

    public SseEmitter sendEmitterStream(){
        emitter = new SseEmitter(180000L);
        cachedThreadPool.execute(() -> {
            try {
                emitter.send(SseEmitter.event()
                       .name("videostream")
                       .data("[{\"data\":\"videostream\","
                               + "\"event\":\"videostream\"}]"));
                emitter.complete();
            } catch (Exception e) {
                emitter.completeWithError(e);
            }
        });
        return emitter;
    }
}
