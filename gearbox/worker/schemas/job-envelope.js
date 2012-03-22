{
    "type": "object",
    "additionalProperties": false,
    "properties": {
        "version": {
            "type": "string",
            "pattern": "^v\\d+$"
        },
        "job_type": {
            "type": "string",
            "enum": ["sync", "async"]
        },
        "operation": {
            "type": "string",
            // would use enum here, except some non mon_gearbox uses might use different
            // operation names
            "options":[{"value":"get"},{"value":"delete"},{"value":"update"},{"value":"create"}]
        },
        "component": {
            "type": "string"
        },
        "status" : {
            "type": "string",
            "optional": true
        },
        "parent_uri" :  {
            "type": "string",
            "format": "uri",
            "optional": true
        },
        "arguments": {
            "type": "array",
            "optional": true,
            "default": []
        },
        "matrix_arguments": {
            "type": "object",
            "optional": true,
            "default": {}
        },
        "remote": {
            "type": "object",
            "optional": true,
            "properties": {
                "ip": {
                    "type": "string",
                    "format": "ipv4",
                    "optional": true
                },
                "user": {
                    "type": "string",
                    "optional": true
                }
            }
        },
        "headers": {
            "type": "object",
            "additionalProperties": {
                "type": "string"
            },
            "optional": true
        },
        "content": {
            "type": "string",
            "optional": true
        },
        "resource": {
            "type": "object",
            "additionalProperties": false,
            "properties": {
                "name": {
                    "type": "string",
                    "optional": true
                },
                "type": {
                    "type": "string"
                }
            }
        },
	    "environ": {
	        "type": "object",
            "additionalProperties": {
                "type": "string"
            },
            "optional": true
	    },
	    "query_params": {
	        "type": "object",
            "additionalProperties": {
                "type": "string"
            },
            "optional": true
	    },
        "base_uri": {
            "type": "string",
            "format": "uri",
            "optional": true
        },
        "on": {
            "type": "object",
            "optional": true,
            "additionalProperties": false,
            "properties": {
                "COMPLETED": {
                    "type": "object",
                    "optional": true,
                    "additionalProperties": false,
                    "properties": {
                        "name": {
                            "type": "string"
                        },
                        "envelope": {
                            "type": "object"
                        }
                    }
                },
                "FAILED": {
                    "type": "object",
                    "optional": true,
                    "additionalProperties": false,
                    "properties": {
                        "name": {
                            "type": "string"
                        },
                        "envelope": {
                            "type": "object"
                        }
                    }
                },
                "SUCCEEDED": {
                    "type": "object",
                    "optional": true,
                    "additionalProperties": false,
                    "properties": {
                        "name": {
                            "type": "string"
                        },
                        "envelope": {
                            "type": "object"
                        }
                    }
                },
                "STARTED": {
                    "type": "object",
                    "optional": true,
                    "additionalProperties": false,
                    "properties": {
                        "name": {
                            "type": "string"
                        },
                        "envelope": {
                            "type": "object"
                        }
                    }
                },
                "STOPPED": {
                    "type": "object",
                    "optional": true,
                    "additionalProperties": false,
                    "properties": {
                        "name": {
                            "type": "string"
                        },
                        "envelope": {
                            "type": "object"
                        }
                    }
                },
                "CANCELLED": {
                    "type": "object",
                    "optional": true,
                    "additionalProperties": false,
                    "properties": {
                        "name": {
                            "type": "string"
                        },
                        "envelope": {
                            "type": "object"
                        }
                    }
                }                    
            }
        },
        "event_status": {
            "type": "object",
            "optional": true
        }
    }
}
