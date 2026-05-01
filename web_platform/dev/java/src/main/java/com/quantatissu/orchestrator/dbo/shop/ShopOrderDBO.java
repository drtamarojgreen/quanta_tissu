package com.quantatissu.orchestrator.dbo.shop;

import com.quantatissu.orchestrator.dbm.HibernateAdmin;
import com.quantatissu.orchestrator.dbo.DatabaseObject;
import com.quantatissu.orchestrator.model.shop.ShopOrder;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

public class ShopOrderDBO extends DatabaseObject {

    public static void saveSQLShopOrder(ShopOrder shopOrder) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "INSERT INTO ShopOrder(customerId, lastModified, datePurchased, orderAmount) VALUES(?,?,?,?)";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setInt(1,shopOrder.getCustomerId());
                pstmt.setString(2,shopOrder.getLastModified());
                pstmt.setString(3,shopOrder.getDatePurchased());
                pstmt.setFloat(4,shopOrder.getOrderAmount());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }

    public static List<ShopOrder> loadShopOrders(){
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            List<ShopOrder> shopOrderList = new ArrayList<>();
            String sql = "SELECT id,customerId,lastModified,datePurchased,orderAmount FROM ShopOrder;";
            try  {
                        conn = DriverManager.getConnection(connectionURL);
                        Statement stmt = conn.createStatement();
                        ResultSet rs = stmt.executeQuery(sql);
                        while(rs.next()){
                           ShopOrder shopOrder = new ShopOrder();
                           shopOrder.setId(rs.getInt("id"));
                           shopOrder.setCustomerId(rs.getInt("customerId"));
                           shopOrder.setLastModified(rs.getString("email"));
                           shopOrder.setDatePurchased(rs.getString("datePurchased"));
                           shopOrder.setOrderAmount(rs.getFloat("orderAmount"));
                           shopOrderList.add(shopOrder);
                       }
                tx.commit();
                session.close();
                return shopOrderList;
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
        return null;
    }
    
}
