package com.quantatissu.orchestrator.service;

import org.springframework.web.multipart.MultipartFile;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.ByteArrayInputStream;

public class DecodedMultipartFile implements MultipartFile {
    private final byte[] imgContent;
    private final String header;

    public DecodedMultipartFile(byte[] imgContent, String header) {
        this.imgContent = imgContent;
        this.header = header;
    }

    @Override public String getName() { return System.currentTimeMillis() + Math.random() + "." + header.split("/")[1]; }
    @Override public String getOriginalFilename() { return System.currentTimeMillis() + (int)Math.random() * 10000 + "." + header.split("/")[1]; }
    @Override public String getContentType() { return header.split(":")[1]; }
    @Override public boolean isEmpty() { return imgContent == null || imgContent.length == 0; }
    @Override public long getSize() { return imgContent.length; }
    @Override public byte[] getBytes() throws IOException { return imgContent; }
    @Override public InputStream getInputStream() throws IOException { return new ByteArrayInputStream(imgContent); }
    @Override public void transferTo(File dest) throws IOException, IllegalStateException { new java.io.FileOutputStream(dest).write(imgContent); }
}
