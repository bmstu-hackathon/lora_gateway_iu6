# coding: utf-8
from flask import Flask, render_template
from peewee import *

# app configuration
app = Flask(__name__)
# app.config.from_object(__name__)
app.debug = True

CAR_PLACE_COUNT = 16
STATUS_FREE = 0
STATUS_RESERVED = 1


# database configuration
db = SqliteDatabase('places.db')

class Place(Model):
    identifier = IntegerField(unique=True)
    status = IntegerField()

    class Meta:
        database = db


@app.route('/')
def status():
    query = Place.select()
    return render_template("index.html", places=query)


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



if __name__ == '__main__':

    if not Place.table_exists():
        Place.create_table(True)

        # Creating car places
        for i in range(1, CAR_PLACE_COUNT + 1):
            Place.create(identifier=i, status=STATUS_FREE)

    app.run()
