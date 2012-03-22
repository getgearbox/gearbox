{
    "type": "object",
    "additionalProperties": false,
    "properties": {
        "operation": {
            "type": "string",
            "options":[{"value":"get"},{"value":"delete"},{"value":"update"},{"value":"create"}]
        },
        "component": {
            "type": "string"
        },
        "state": {
            "type": "string",
            "enum":["PENDING","RUNNING","CANCELLING","CANCELLED","COMPLETED"]
        },
        "uri": {
            "type": "string",
            "format": "uri"
        },
        "status_uri": {
            "type": "string",
            "format": "uri",
            "optional": true
        },
        "progress": {
            "type": "integer",
            "minimum": 0,
            "maximum": 100
        },
        "code": {
            "type": "integer",
            "minimum": 0,
            "optional": true
        },
        "parent_uri": {
            "type": "string",
            "format": "uri",
            "optional": true
        },
        "messages": {
            "type": "array",
            "items": {
                "type": "string"
            }
        },
        "children": {
            "type": "array",
            "items": {
                "type": "string",
                "format": "uri"
            },
            "optional": true
	    },
        "failures": {
            "type": "integer",
            "optional": true,
            "minimum": 0,
            "description": "the count of how many times the job has been retried"
        },
	    "ctime": {
            "type": "integer",
            "optional": true,
            "minimum": 0
        },
	    "mtime": {
            "type": "integer",
            "optional": true,
            "minimum": 0
        },
	    "ytime": {
            "type": "integer",
            "optional": true,
            "minimum": 0,
            "description": "yield time, when it is scheduled to start running again"
        },
        "concurrency": {
            "type": "integer",
            "optional": true,
            "minimum": 0,
            "description": "count of how many processes are concurrently working on this status"
        }
    }
}
