package com.quantatissu.orchestrator.dbo.integrations;

import com.quantatissu.orchestrator.dbm.HibernateAdmin;
import com.quantatissu.orchestrator.dbo.DatabaseObject;
import com.quantatissu.orchestrator.model.integrations.SocialMedia;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;


public class SocialMediaDBO extends DatabaseObject{

    public static void saveSocialMedia(SocialMedia socialMedia){
        try {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            session.save(socialMedia);
            session.flush();
                tx.commit();
                session.close();
        } catch (Exception ex) {
            System.out.println(ex.getMessage());
            ex.printStackTrace();
        }
    }

    public static void saveSQLSocialMedia(SocialMedia socialMedia) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "INSERT INTO SocialMedia(uid, accessToken, responseEmail) VALUES(?,?,?)";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setString(1,socialMedia.getUid());
                pstmt.setString(2,socialMedia.getAccessToken());
                pstmt.setString(3,socialMedia.getResponseEmail());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }

    public static void updateSocialMedia(SocialMedia socialMedia) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "UPDATE SocialMedia SET uid = ?, accessToken = ?, responseEmail = ? WHERE id = ?";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setString(1,socialMedia.getUid());
                pstmt.setString(2,socialMedia.getAccessToken());
                pstmt.setString(3,socialMedia.getResponseEmail());
                pstmt.setInt(4,socialMedia.getId());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }

    public static List<SocialMedia> loadSocialMedias(){
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            List<SocialMedia> socialMediaList = new ArrayList<>();
            String sql = "SELECT id, uid, accessToken, responseEmail FROM SocialMedia;";
            try {
                conn = DriverManager.getConnection(connectionURL);
                Statement stmt = conn.createStatement();
                ResultSet rs = stmt.executeQuery(sql);
                       while(rs.next()){
                           SocialMedia socialMedia = new SocialMedia();
                           socialMedia.setId(rs.getInt("id"));
                           socialMedia.setUid(rs.getString("uid"));
                           socialMedia.setAccessToken(rs.getString("accessToken"));
                           socialMedia.setResponseEmail(rs.getString("responseEmail"));
                           socialMediaList.add(socialMedia);
                       }
                tx.commit();
                session.close();
                return socialMediaList;
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
        return new ArrayList<SocialMedia>();
    }

    
    
}
