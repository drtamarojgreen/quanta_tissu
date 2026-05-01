package com.quantatissu.orchestrator.repository.conference;

import com.quantatissu.orchestrator.model.conference.Room;
import org.springframework.data.jpa.repository.JpaRepository;
import org.springframework.stereotype.Repository;

@Repository
public interface RoomRepository extends JpaRepository<Room, Integer> {
}
