# coding: utf-8
from peewee import *

# Конфигурация подключения SQLite базы данных к приложению
db = SqliteDatabase('places.db')


CAR_PLACE_COUNT = 16
STATUS_FREE = 0
STATUS_RESERVED = 1

# Класс, описывающий структуру данных в БД: identifier - номер парковочного места, status - его статус
class Place(Model):
    identifier = IntegerField(unique=True)
    status = IntegerField()

    class Meta:
        database = db

# Метод, осуществляющий обновление парковочного места в базе данных
def update_status(identifier, status):
    if status == STATUS_FREE or status == STATUS_RESERVED:
        if  1 << identifier << CAR_PLACE_COUNT:
            query = Place.update(status=status).where(Place.identifier == identifier)
            query.execute()
        else:
            return False
    else:
        return False

    return True
