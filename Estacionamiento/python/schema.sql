-- Esquema de base de datos para el sistema de estacionamiento
-- Crear base de datos
CREATE DATABASE IF NOT EXISTS estacionamiento_db;
USE estacionamiento_db;

-- Tabla de lecturas de sensores
CREATE TABLE IF NOT EXISTS sensor_readings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    distance FLOAT,
    uptime_seconds INT,
    available_slots INT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Tabla de eventos de barrera (entrada/salida)
CREATE TABLE IF NOT EXISTS barrier_events (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    barrier_type ENUM('entrance', 'exit') NOT NULL,
    event_type ENUM('raised', 'lowered') NOT NULL,
    reason VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp),
    INDEX idx_barrier_type (barrier_type)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Tabla de accesos RFID
CREATE TABLE IF NOT EXISTS rfid_access (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    card_uid VARCHAR(50) NOT NULL,
    authorized BOOLEAN NOT NULL,
    action VARCHAR(100),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp),
    INDEX idx_card_uid (card_uid),
    INDEX idx_authorized (authorized)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Tabla de cambios de configuración
CREATE TABLE IF NOT EXISTS config_changes (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    parameter_name VARCHAR(100) NOT NULL,
    old_value INT,
    new_value INT,
    changed_by VARCHAR(50),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp),
    INDEX idx_parameter (parameter_name)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Tabla de ocupación de espacios (histórico)
CREATE TABLE IF NOT EXISTS slot_occupancy (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    slot_number INT NOT NULL,
    occupied BOOLEAN NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp),
    INDEX idx_slot (slot_number)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- Tabla de alertas y eventos del sistema
CREATE TABLE IF NOT EXISTS system_events (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    event_type VARCHAR(100) NOT NULL,
    severity ENUM('info', 'warning', 'error', 'critical') DEFAULT 'info',
    message TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_timestamp (timestamp),
    INDEX idx_severity (severity)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
