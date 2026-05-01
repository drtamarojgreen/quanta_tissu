package com.quantatissu.orchestrator.model.integrations;

import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.Table;
import org.hibernate.search.annotations.Indexed;

@Entity
@Indexed
@Table(name = "SocialMedia")
public class SocialMedia {

        @Id
    @GeneratedValue(strategy = GenerationType.AUTO)
    private Integer id;
    String uid;
    String accessToken;
    String responseEmail;

    public SocialMedia(){
    }
    
    public SocialMedia(String uid, String accessToken, String responseEmail){
        this.uid = uid;
        this.accessToken = accessToken;
        this.responseEmail = responseEmail;
    }

    public Integer getId() {
        return id;
    }

    public void setId(Integer id) {
        this.id = id;
    }

    public String getUid() {
        return uid;
    }

    public void setUid(String uid) {
        this.uid = uid;
    }

    public String getAccessToken() {
        return accessToken;
    }

    public void setAccessToken(String accessToken) {
        this.accessToken = accessToken;
    }

    public String getResponseEmail() {
        return responseEmail;
    }

    public void setResponseEmail(String responseEmail) {
        this.responseEmail = responseEmail;
    }
    
}
