{
    "type": "object",
    "additionalProperties": false,
    "properties": {
        "status": {
            "type": "integer",
            "minimum": 0,
            "maximum": 1024
        },
        "path": {
            "type": "string"
        },
        "method": {
            "type": "string",
            "enum": ["GET", "POST", "PUT", "DELETE"]
        },
        "message": {
            "type": "string"
        }
    }
}