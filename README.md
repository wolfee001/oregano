# oregano

Message broker based messaging library for event (pub/sub) and transaction (request/response) system

## What does it provide?

`oregano` does not force any communication protocol or message format. Any of these definition must take place in the application layer. `oregano` only provides end-to-end communication system regardless the message. Any extra header over the message that is needed for the library added after the entry point of the message and removed before the exit point of the message. They are not visible for the application layer.

### Event system

The event system implements a publisher - subscriber system. Any publisher can publish onto any channel and any subscriber can subscribe to any channel. An event published on a given channel will be received by all subscribers subscribed to the channel. The message itself can be any bytestream represented as `std::string`. In the internals the message will be transformed to `base64` string.

<!-- ```plantuml
@startuml

skinparam linetype polyline

component service_a as "service a" {
    rectangle publisher_logic1 as "publisher logic"

    package IEventInterface1 as "IEventInterface" {
        rectangle IPublisher1 as "IPublisher"
    }
}

component service_b as "service b" {
    rectangle publisher_logic2 as "publisher logic"

    package IEventInterface2 as "IEventInterface" {
        rectangle IPublisher2 as "IPublisher"
    }
}

cloud message_broker as "message broker" {
    cloud channel_1
    cloud channel_2
    channel_1 -d[hidden]- channel_2
}

component service_c as "service c" {
    rectangle subscriber_logic1 as "subscriber logic"

    package IEventInterface3 as "IEventInterface" {
        rectangle ISubscriber1 as "ISubscriber"
    }
}

component service_d as "service d" {
    rectangle subscriber_logic2 as "subscriber logic"

    package IEventInterface4 as "IEventInterface" {
        rectangle ISubscriber2 as "ISubscriber"
    }
}

component service_e as "service e" {
    rectangle subscriber_logic3 as "subscriber logic"

    package IEventInterface5 as "IEventInterface" {
        rectangle ISubscriber3 as "ISubscriber"
    }
}

service_a -d[hidden]- service_b

service_c -d[hidden]- service_d
service_d -d[hidden]- service_e

publisher_logic1 -> IPublisher1 : raw message
IPublisher1 =[#red]> channel_1 : base64 message

publisher_logic2 -> IPublisher2 : raw message
IPublisher2 =[#blue]> channel_2 : base64 message

channel_1 =[#red]> ISubscriber1 : base64 message
ISubscriber1 -> subscriber_logic1 : raw message

channel_1 =[#red]> ISubscriber2 : base64 message
ISubscriber2 -> subscriber_logic2 : raw message

channel_2  =[#blue]> ISubscriber2 : base64 message

channel_2 =[#blue]> ISubscriber3 : base64 message
ISubscriber3 -> subscriber_logic3 : raw message

@enduml
```-->

![Pubsub](https://www.plantuml.com/plantuml/svg/dLKzRy8m4DtzAwnqPi2X7P2AM3gmLUg8KEIFAqGuJcGdLAZYlzVasyI44ZO_lxjtxpXVrYQdEYyIYP0vnYgZcYPOnWhoSmOuI-NvUa28fqcMAb0vDg1FCOU8OcfmK9ym3V0lmlQdWUTKxQKDBfYCpG5q9DDzpESblmLn2GQe3CeeFz8zuCt7oKfiL0xwctAeGcx0HiWLstmsYUkWzXoKx0kw-4mmnmHxQ88yRu9CCK56JCYq43W1OwnOn7Hwj7LSsJM4AwXHgEZyG9K26Sqz6AcmXe5dOdk8XG2rc_KutlPnftrys3vJCCDrpDmXwD3_D31yee5VRUPw33fWT0w4OqGyDa8c65bCCaAUCWAE4NXi99nWv7MIaN34IBTK-jFNljAEmBq4WRe_qdTl5zXW6yrMpj9OOar_ccU5-ZVlsnSDOhVgFP4bPjJ0swBbey6USDAJk-d9DJsJ1VJo4q_-JhijnndsGOHpQuiPlaItf0SAP5I1-1J8FGM2UwR7DNentf1mjApGLrPuMzOQbB0Vm3y0 "Pubsub")

### Transaction system

The transaction system implements a request - response pattern. The request is sent to a queue and the response is returned via an event. The request data first get base64 encoded then an extra header applied:

```json
{
  "header": {
    "response_channel": "transaction interface unique channel id",
    "id": "unique message id",
    "valid_until": "a timepoint until the request is valid"
  },
  "payload": "base64 encoded application layer request"
}
```

Then the whole message frame gets base64 encoded and sent through the broker.

The response is sent back as event to the `header.response_channel` with the following message frame:

```json
{
  "header": {
    "id": "the request's id"
  },
  "payload": "base64 encoded application layer response"
}
```

The whole response is sent in base64 encoded format through the channel.

The `TransactionInterface`-unique `response_channel` ensures that only the request issuer gets the response. The `id` ensures that only the corresponding `RequestPromise` will be notified.

The request must sent to a queue to ensure that only one consumer will receive the given request. This also acts a natural load balancer without an additional service, since the consumer will only try pick the next message when it's ready to process. If the given message broker does not support queues, the queue functionality must emulated with other facilities (like subscriber group in MQTT5).

<!-- ```plantuml
@startuml

skinparam linetype polyline

component producer as "producer" {
    rectangle producer_logic as "producer logic"

    package ITransactionInterface1 as "ITransactionInterface" {
        rectangle IRequestSender
        package IEventInterface1 as "IEventInterface" {
            rectangle ISubscriber
        }
    }

    IRequestSender -d[hidden]- IEventInterface1
}

cloud message_broker as "message broker" {
    queue consumer_queue
    cloud AttaCaccA_CACaGa
    consumer_queue -d[hidden]- AttaCaccA_CACaGa
}

component consumer as "consumer" {
    package ITransactionInterface2 as "ITransactionInterface" {
        rectangle IRequestHandler
        package IEventInterface2 as "IEventInterface" {
            rectangle IPublisher
        }
    }

    rectangle consumer_logic as "consumer logic"

    IRequestHandler -d[hidden]- IEventInterface2

    ITransactionInterface2 -d[hidden]- consumer_logic
}

producer_logic =[#red]> IRequestSender : raw request
IRequestSender =[#red]> consumer_queue : base64 encoded\nrequest with\nheader
consumer_queue =[#red]> IRequestHandler : base64 encoded\nrequest with\nheader
IRequestHandler =[#red]> consumer_logic : raw request

consumer_logic =[#blue]> IPublisher : raw response
IPublisher =[#blue]> AttaCaccA_CACaGa : base64 encoded\nresponse with\nheader
AttaCaccA_CACaGa =[#blue]> ISubscriber : base64 encoded\nresponse with\nheader
ISubscriber =l[#blue]> producer_logic : raw response

@enduml
``` -->

![Transaction](https://www.plantuml.com/plantuml/svg/dLHDJyCm3BtdLrZZp849SK02CKq8TaF03T3a9cQhbYOb7o24-EzaxPerwHWVEtIAxVVyukVsmZeqpbUICRigLOq6Av2b8lTU4zHQlgyFZ75TrLgHSb0RBJmd0sXXr1r6yC4W_0nnXseXAPRDfLwKF2c69ZHY3Q96li85mUpUeBB8NQdLJ3aopyZfgC7jJ7KTqwwpMthnPDqTAK4c5iGcbw_X2Zbz6kmJP-Htlh3Sb4MF-PErp-OlxGw7uc5P2a7gwN3GcGK8bzeBgCZQe6rU6BtQJ7KJWZRK2GhKde1hPNqLvjeScqJBCt4EfyZvP3wTJF4AsrHIdIWQr7_sNUw0ZPxkq2dPwzdulvvTenBoPzF6VpJjnXUojCllFDjMnbbjDpPEeR-ncTvzDeytYDrpwWFJvciliXVexE70a7WwpvVi50o-XLiqCPOb8oZRWrCeqDB9CP3YMf1uL1i2U2lTyb4j2TUlJuOQIEWcy4kw73PKrrurlHBBiW5LI4zh6T7Q2B5XVIsnNcPRdY_yJjajGQfx0En9s7uIVatNnvp9I9NvdTs8NGH7msVw2m00 "Transaction")

## How to use

For usage check the examples

## Message brokers

At the time being the only message broker that is implemented is `Redis`. However any message broker can be used if it can implement `IMesageBrokerConnector` interface and the concept of `publish / subscribe` and `queue` (or subscriber group like MQTT5).

### Brokers worth to implement

- [x] Redis
- [ ] MQTT5 (Mosquitto / HiveMQ)
- [ ] RabbitMQ
- [ ] Kafka
