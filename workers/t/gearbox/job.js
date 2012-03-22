{
    "version": "v1",
    "operation": "create",
    "component": "thing",
    "arguments": ["t-1234"],
    "status": "s-1234",
    "matrix_arguments":{"arg1":"value1"},
    "remote":{"user": "dmuino","ip":"1.2.3.4"},
    "request":{
        "header1": "value1",
        "_content": "{\"var\":\"value\"}"
    },
    "resource":{"type":"thing","name":"t-1234"},
    "environ":{"env1":"value1"},
    "query_params":{"param1":"value1"},
    "transaction":{
        "uri":"http://host:4080/tranaction/v1/t-1234",
        "state":"setup",
        "progress": 0,
        "accept": 0
    },
    "base_uri": "http://host:4080/thing/v1"
}