package com.quantatissu.orchestrator.dbo.shop;

import com.quantatissu.orchestrator.dbm.HibernateAdmin;
import com.quantatissu.orchestrator.dbo.DatabaseObject;
import com.quantatissu.orchestrator.model.shop.ShopPaymentType;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

public class ShopPaymentTypeDBO extends DatabaseObject {

    public static void saveSQLShopPaymentType(ShopPaymentType shopPaymentType) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "INSERT INTO ShopPaymentType(paymentTypeDescription) VALUES(?)";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setString(1,shopPaymentType.getPaymentTypeDescription());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }

    public static void updateShopPaymentType(ShopPaymentType shopPaymentType) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "UPDATE ShopPaymentType SET paymentTypeDescription = ? WHERE id = ?";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setString(1,shopPaymentType.getPaymentTypeDescription());
                pstmt.setInt(2,shopPaymentType.getId());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }
    
    
    public static List<ShopPaymentType> loadShopPaymentTypes(){
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            List<ShopPaymentType> shopPaymentTypeList = new ArrayList<>();
            String sql = "SELECT id,paymentTypeDescription FROM ShopPaymentType;";
            try  {
                        conn = DriverManager.getConnection(connectionURL);
                        Statement stmt = conn.createStatement();
                        ResultSet rs = stmt.executeQuery(sql);
                        while(rs.next()){
                           ShopPaymentType shopPaymentType = new ShopPaymentType();
                           shopPaymentType.setId(rs.getInt("id"));
                           shopPaymentType.setPaymentTypeDescription(rs.getString("description"));
                           shopPaymentTypeList.add(shopPaymentType);
                       }
                tx.commit();
                session.close();
                return shopPaymentTypeList;
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
        return null;
    }
    
}
