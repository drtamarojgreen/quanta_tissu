package com.quantatissu.orchestrator.dbo.shop;

import com.quantatissu.orchestrator.dbm.HibernateAdmin;
import com.quantatissu.orchestrator.dbo.DatabaseObject;
import com.quantatissu.orchestrator.model.shop.ShopItem;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

public class ShopItemDBO extends DatabaseObject {

    public static void saveSQLShopItem(ShopItem shopItem) {
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            String sql = "INSERT INTO ShopItem(productId, quantity) VALUES(?,?)";
            try {
                conn = DriverManager.getConnection(connectionURL);
                pstmt = conn.prepareStatement(sql);
                pstmt.setInt(1,shopItem.getProductId());
                pstmt.setInt(2,shopItem.getQuantity());
                pstmt.executeUpdate();
                tx.commit();
                session.close();
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
    }

    public static List<ShopItem> loadShopItems(){
            session = HibernateAdmin.getSession();
            tx = session.beginTransaction();
            List<ShopItem> shopItemList = new ArrayList<>();
            String sql = "SELECT id,productId,quantity FROM ShopItem;";
            try  {
                        conn = DriverManager.getConnection(connectionURL);
                        Statement stmt = conn.createStatement();
                        ResultSet rs = stmt.executeQuery(sql);
                        while(rs.next()){
                           ShopItem shopItem = new ShopItem();
                           shopItem.setId(rs.getInt("id"));
                           shopItem.setProductId(rs.getInt("productId"));
                           shopItem.setQuantity(rs.getInt("quantity"));
                           shopItemList.add(shopItem);
                       }
                tx.commit();
                session.close();
                return shopItemList;
            } catch (SQLException e) {
                    System.out.println(e.getMessage());
            tx.rollback();
            }
        return null;
    }
    
}
