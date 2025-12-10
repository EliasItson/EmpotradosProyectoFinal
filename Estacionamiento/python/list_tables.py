import mysql.connector
from mysql.connector import Error

try:
    conn = mysql.connector.connect(host='localhost', user='root', password='root', database='estacionamiento_db')
    cur = conn.cursor()
    cur.execute("SHOW TABLES")
    rows = cur.fetchall()
    print('Tables in estacionamiento_db:')
    for r in rows:
        print(' -', r[0])
    cur.close()
    conn.close()
except Error as e:
    print('ERROR:', e)
