package com.quantatissu.orchestrator.controller.conference;

import com.quantatissu.orchestrator.model.conference.Room;
import com.quantatissu.orchestrator.repository.conference.RoomRepository;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;
import java.util.List;

@RestController
@RequestMapping("/api/conference")
public class ConferenceController {

    @Autowired
    private RoomRepository roomRepository;

    @GetMapping("/rooms")
    public List<Room> getAllRooms() {
        return roomRepository.findAll();
    }

    @PostMapping("/rooms")
    public Room createRoom(@RequestBody Room room) {
        return roomRepository.save(room);
    }

    @GetMapping("/rooms/{id}")
    public Room getRoom(@PathVariable Integer id) {
        return roomRepository.findById(id).orElse(null);
    }

    @DeleteMapping("/rooms/{id}")
    public void deleteRoom(@PathVariable Integer id) {
        roomRepository.deleteById(id);
    }
}
