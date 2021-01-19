# edge-sql

![Deploy](https://github.com/lspgn/edge-sql/workflows/Deploy/badge.svg)

A [Cloudflare Worker](https://workers.cloudflare.com/) embedding [SQLite](https://sqlite.org/)
with [WASM](https://webassembly.org/) and a [simple Forex dataset](#data).

You can preview it here: [https://sql.lspgn.workers.dev](https://sql.lspgn.workers.dev).

## Why?

_I had an idea. Here's a little backstory._

Many services provide a REST API on top of a SQL database.
For certain datasources, flexibility in terms of queries is key.
Often those use-cases are dashboard prototyping, for example
with [Grafana](https://en.wikipedia.org/wiki/Grafana).

This data is usually read-only statistical datasets where the user
needs to run many queries in order to troubleshoot an issue
or just find the best visualization.
Often this requires complex queries and extensive accesses or quotas.
Using regular [REST APIs](https://en.wikipedia.org/wiki/Representational_state_transfer)
would require a lot of back-and-forth between the developers and the analysts.
Recent initiatives like [GraphQL](https://en.wikipedia.org/wiki/GraphQL)
are aimed at these issues and hope to provide more flexibility.

The idea behind this fun project is that serverless concepts can be applied for this use-case
and have [SQL](https://en.wikipedia.org/wiki/SQL) as an execution model.
[Foreign Data Wrappers](https://wiki.postgresql.org/wiki/Foreign_data_wrappers) are
similar as they provide a framework that works with a Query Processor but 
need to rely on more traditional user control and quotas configurations.

Currently, tools like [BigQuery](https://en.wikipedia.org/wiki/BigQuery)
are used in [multi-tenancy environments](https://en.wikipedia.org/wiki/Multitenancy)
and allow user to execute their SQL program without worrying about data infrastructure.

Through this fun prototype on the Cloudflare Workers platform,
we can put a sandbox around a SQL program and play with data
stored in [Worker KVs](https://developers.cloudflare.com/workers/runtime-apis/kv).

The current setup is meant to use minimal resources ([Free limits](https://developers.cloudflare.com/workers/platform/limits)).
This is a proof of concept that was made possible by SQLite code that can fit inside the plateform (<1MB).
Obviously, a rework of the code would be required to make it usable in a production
environment. Features like sharding the data and the querying
are necessary for large datasets, along with user control. The current quotas are controlled by the platform
which can stop the execution of the entire Worker if it goes above the allowed limits.

## Data

The data used in production is a reduced version (only EUR, JPY, GBP and CHF currencies) of the
[European Central Bank Forex Rates on Humdata](https://data.humdata.org/dataset/ecb-fx-rates).
And is [distributed](https://data.humdata.org/about/license) under the license
[CC-BY](https://creativecommons.org/licenses/by/4.0/legalcode).

## Try

The following query will return the days when the [British pound](https://en.wikipedia.org/wiki/Pound_sterling)
was at its lowest and highest against the dollar.

```bash
$ curl -XPOST --data \
"
SELECT *,1/EUR,1/JPY,1/GBP,1/CHF
FROM forex
WHERE
  GBP = (
    SELECT
      max(GBP)
    FROM forex)
  OR GBP = (
    SELECT
      min(GBP)
    FROM forex)
" \
-H 'content-type: application/text' \
https://sql.lspgn.workers.dev/
Date,EUR,JPY,GBP,CHF,1/EUR,1/JPY,1/GBP,1/CHF
2007-11-08,1.4666,0.008840265220012,2.10642728904847,0.883440756580929,0.681849174962498,113.118778126279,0.47473748806764,1.13193781535524
```

[Builtin SQLite functions](https://sqlite.org/lang_corefunc.html)
along with extra features are avaialble.

```bash
$ curl -XPOST --data \
"
SELECT
  getdata('country') AS country,
  random() AS rnd,
  date('now') AS now
" \
-H 'content-type: application/text' \
https://sql.lspgn.workers.dev/
country,rnd,now
US,-7348224717915799868,2021-01-18
```
