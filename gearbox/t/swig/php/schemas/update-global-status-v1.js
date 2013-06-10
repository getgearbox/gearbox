{
    "type": "object",
    "additionalProperties": false,
    "properties": {
        "state": {
            "type": "string",
            "enum": [ "CANCELLED" ],
            "optional": true
        },
        "ytime": {
            "type": "integer",
            "minimum": 0,
            "optional": true
        },
        "children": {
            "type": "array",
            "minimum": 0,
            "optional": true,
            "items": {
                "type": "string",
                "format": "uri"
            }
        },
        "progress": {
            "type": "integer",
            "minimum": 0,
            "maximum": 100,
            "optional": true
        }
    }
}
