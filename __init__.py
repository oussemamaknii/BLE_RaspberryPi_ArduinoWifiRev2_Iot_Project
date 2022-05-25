from datetime import datetime
from bson import ObjectId
from fastapi.encoders import jsonable_encoder
from pydantic import BaseModel, Field
from typing import Optional
from datetime import datetime
from pymongo.collection import Collection
from flask import Flask, jsonify
from flask_pymongo import PyMongo
from pymongo.errors import DuplicateKeyError


class Gps(BaseModel):
    lon: float
    lat: float


class Sensors(BaseModel):
    id: Optional[PydanticObjectId] = Field(None, alias="_id")
    mac_id: str
    gps: Gps
    step: int
    time: Optional[datetime]
    fallen: bool

    def to_json(self):
        return jsonable_encoder(self, exclude_none=True)


class PydanticObjectId(ObjectId):
    """
    Object Id field. Compatible with Pydantic.
    """

    @classmethod
    def __get_validators__(cls):
        yield cls.validate

    @classmethod
    def validate(cls, v):
        return PydanticObjectId(v)

    @classmethod
    def __modify_schema__(cls, field_schema: dict):
        field_schema.update(
            type="string",
            examples=["5eb7cf5a86d9755df3a6c593", "5eb7cfb05e32e07750a1756a"],
        )


app = Flask(__name__)
app.config["MONGO_URI"] = "mongodb://localhost:27017/IotDB"
pymongo = PyMongo(app)

sensors: Collection = pymongo.db.Sensors


@app.errorhandler(404)
def resource_not_found(e):
    return jsonify(error=str(e)), 404


@app.errorhandler(DuplicateKeyError)
def resource_not_found(e):
    return jsonify(error=f"Duplicate key error."), 400


@app.route("/sensors/")
def list_sensors():

    cursor = sensors.find().sort("time")

    return {
        "sensors": [Sensors(**doc).to_json() for doc in cursor]
    }


app.run(host='0.0.0.0', port=8090)
