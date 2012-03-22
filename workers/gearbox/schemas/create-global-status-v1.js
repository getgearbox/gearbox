{
    "type": "object",
    "additionalProperties": false,
    "properties": {
        "name": {
            "type": "string"
        },
        "operation": {
            "type": "string"
        },
        "uri": {
            "type": "string",
            "format": "uri"
        },
        "component": {
            "type": "string"
        },
        "parent_uri": {
            "type": "string",
            "format" : "uri",
            "optional": true
        }
    }
}
