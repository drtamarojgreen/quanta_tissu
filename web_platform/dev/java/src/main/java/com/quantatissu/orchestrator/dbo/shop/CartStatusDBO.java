package com.quantatissu.orchestrator.dbo.shop;

import com.quantatissu.orchestrator.dbm.HibernateAdmin;
import com.quantatissu.orchestrator.dbo.DatabaseObject;
import com.quantatissu.orchestrator.model.hr.HrGroup;
import com.quantatissu.orchestrator.model.shop.CartStatus;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

public class CartStatusDBO extends DatabaseObject {

    public static void saveSQLCartStatus(CartStatus cartStatus) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "INSERT INTO CartStatus(description) VALUES(?)";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setString(1,cartStatus.getDescription());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }

    public static void updateCartStatus(CartStatus cartStatus) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "UPDATE CartStatus SET description = ? WHERE id = ?";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setString(1,cartStatus.getDescription());
                pstmt.setInt(2,cartStatus.getId());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }
    
    
    public static List<CartStatus> loadCartStatuses(){
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            List<CartStatus> cartStatusList = new ArrayList<>();
            String sql = "SELECT id,description FROM CartStatus;";
            try  {
                        conn = DriverManager.getConnection(connectionURL);
                        Statement stmt = conn.createStatement();
                        ResultSet rs = stmt.executeQuery(sql);
                        while(rs.next()){
                           CartStatus cartStatus = new CartStatus();
                           cartStatus.setId(rs.getInt("id"));
                           cartStatus.setDescription(rs.getString("description"));
                           cartStatusList.add(cartStatus);
                       }
                tx.commit();
                session.close();
                return cartStatusList;
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
        return null;
    }
    
}
