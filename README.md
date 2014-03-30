leveldbd
========

LevelDBd is a key/value database server which provides REST API and RPC(Protocol Buffer) service backed up with LevelDB engine.

# Background

Google published a embedded key/value store engine called LevelDB under BSD license.
It's a sort of BigTable implementation and could be sit as a core database engine on NoSQL style database service.
The biggest difference with traditional/other key/value stores like Berkerly DB is that it manages keys in sorted order, this is a greate thing because we can run list operation to get a sorted list of keys along with values that we couldn't so far with other embedded key/value engines, so this feature adds great and great value to LevelDB and make it so suitable to be adopted for NoSQL database service.

LevelDBd uses LevelDB as a back-end db engine and provides remote server functions.
It provides 2 ways of interface:
* REST API as a general purpose interface - Any HTTP client library or web browser can be used to communicate.
* RPC as a programatic interface to maximize the performance. (Not supported yet)

In REST API, it provides 4 key features:
* GET for List of keys
  * Supports iteration with maximum number of keys to list for a single request.
  * Supports delimiter and key filter with regular expression.
* GET/PUT
  * Supports encoding
* DELETE
  * Supports single key deletion, prefix deletion and batch deletion.

## LevelDBCluster?
LevelDBCluster is a fault torrent key/value database based on quorum algorithm and it's one of practical BigTable implementation.

To help understand, simply LevelDBServer is like an individual MySQL server which doesn't provide server level redundancy so if the server fails we loose the data. LevelDBCluster is like a MySQL Cluster that runs many back-end MySQL instances. LevelDBCluster cooperates with many back-end LevelDBServer and provides productionl level rock-solid stability and robustiness along with partitioning feature for infinite scalability. It runs seamlessly regardless of individual server failures. LevelDBCluster is not published yet on the internet but you could contact the author if you're adventerus.


### What about Cassandra architecture?

It depends but Cassandra is not really scaling up to a very large scale. Cassandra provides lesser administration cost since there's no partitioning work. Basically Cassandra uses a hash-based ring and distribute rows to corresponding to the host that's owning that particular hash range. So as you add more server, more even distribution you will get. But if you have a really large table, indexes will become a bottle-neck that will drag it down scaling up because you have only a few servers that will store the index. When you send a range or list query on your keys, it only goes to those a fews. If your applicataion only needs exact key match operation, it's perfect but for range and list operation will become a one biggest challenge of managing Cassandra cluster as the number of keys grows. So you'll look for a work-around. So don't be pooled by the benchmark output such as Netflix's that measures only write throughput if you'll need range and list query.

Cassandra is one of beautiful arts in this area but what I'm saying is that you need to understand the both good sides and bad sides. So bad side? It sounds like you'll never need a partitioning technique but just adding new server will make it scale up magically? Wrong. For a really large scale, you'll need to do partitioning on your application level by yourself. That story only works for a small low TPS database.

One opposite architecture of hash-ring is range partitioning, your keys will be stored on specific range of partition. It resolves above scale issue but there'll be a management work on splitting/merging partitions, but the amount of effort will be lesser than you put on Cassandra model if your dataset is really large like Google. Good thing is you have a control of heat partition and it isolates failures. And in this architecture, there's no need of managing indexes separately. So here's LevelDBCluster, partition-based true scale key-value sture..

# REST API service

## LIST API

List all keys in sorted order that are prefixed by a given key or within the given range.
```
GET /db/key/ HTTP/1.1

200 OK
Content-Type: text/plain
Content-Length: 100
x-key: key
x-size: 3
x-format: text
x-encoding: url
x-delimiter: (Not shown if not specified in the request)
x-keyfilter: (Not shown if not specified in the request)
x-rangekey: (Now shown if not specified in the request)
x-iterator: (Not shown if no iteration is required)

keyA=value
keyB=value
keyC=value
```

For the next iteration. (Also can be given as an option)
```
GET /db/key/#iterator HTTP/1.1
```

Options can be given in 2 ways
```
GET /db/key/?option1=value&option2=value&... HTTP/1.1

Or

GET /db/key/ HTTP/1.1
x-options: option1=value,option2=value,...
```

List of Options
* maxkey : Maximum number of keys (default: 1000)
* format : text | html | xml | json (default: text)
* keyonly : true | false (default false)
* encoding : url | hex | base64 (default: url)
* rangekey : last key for range request (if given perform until the specified key range, if not given perform prefix listing)
* delimiter : character for the boundary (ex: "/")
* keyfilter : regular expression to filter keys.
* iterator : Used for iteration

Response Codes
* 200 : Ok.
* 400 : Bad request.
* 500 : Internal server error

Response Headers
* Content-Type
* Content-Length
* x-key
* x-size
* x-format
* x-encoding
* x-iterator: Will be set if list is truncated by maxkey.


## GET API

Get the value of a key.
```
GET /db/key HTTP/1.1

200 OK
Content-Type: application/octet-stream
Content-Length: 32
x-key: /db/key
x-encoding: raw

(raw data)
```

List of Options
* encoding : raw | url | hex | base64 | gzip (default: raw)
* contenttype : override default content-type to user defined (default: application/octet-stream for 'raw' and 'gzip' incoding, plain/text for other 'encoding')

Response Codes
* 200 : Ok.
* 400 : Bad request.
* 404 : Key not found.
* 500 : Internal server error

Response Headers
* Content-Type
* Content-Length
* x-key
* x-options


## PUT API

Put a key
```
PUT /db/key HTTP/1.1
Content-Length: 32

(raw data)

204 OK
Content-Length: 32
x-key: /db/key
```

List of Options
* encoding : raw | gzip (default: raw)

Response Codes
* 204 : Successfully stored.
* 400 : Bad request.
* 404 : Key not found.
* 500 : Internal server error

Response Headers
* Content-Length
* x-key

## DELETE API

Delete a key
```
DELETE /db/key HTTP/1.1

204 OK
Content-Length: 32
x-key: /db/key
```

Delete all keys starting with key prefix.
```
DELETE /db/key_prefix/ HTTP/1.1

200 OK
Content-Length: 32
x-key: /db/key_prefix
x-size: 3
x-encoding: url
x-iterator:

200 /db/key_prefix_A
200 /db/key_prefix_B
500 /db/key_prefix_C
```

Delete multiple keys.
```
DELETE / HTTP/1.1
x-key: /db/key_A
x-key: /db/key_B, /db/key_C

200 OK
Content-Length: 32
x-key: /
x-size: 3
x-encoding: url
x-iterator:

200 /db/key_A
404 /db/key_B
200 /db/key_C
```


List of Options
* maxkey : Maximum number of keys to delete (default: 1000)
* format : text | html | xml | json (default: text)
* encoding url | hex | base64 (default: url)
* iterator : Used for iteration

Response Codes
* 200 or 204 : Successfully deleted.
* 400 : Bad request.
* 404 : Key not found.
* 500 : Internal server error

Response Headers
* Content-Length
* x-key
```
