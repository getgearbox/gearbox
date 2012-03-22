{
    "type": "object",
    "additionalProperties": false,
    "properties": {
        "name": {
            "type": "string",
            "minLength": 1
        },
        "status_name": {
            "type": "string",
            "optional": true
        },
        "envelope": {
            "type": "object"
        },
        "time": {
            "type": "integer",
            "minimum": 0
        }
    }
}