// import the emscripten glue code
import emscripten from './build/module.js'

addEventListener('fetch', event => {
  event.respondWith(handleRequest(event))
})

let emscripten_module = new Promise((resolve, reject) => {
  emscripten({
    instantiateWasm(info, receive) {
      let instance = new WebAssembly.Instance(wasm, info)
      receive(instance)
      return instance.exports
    },
  }).then(module => {
    resolve({
      query: module.cwrap('query', 'string', ['string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string']),
      module: module,
    })
  })
})

async function handleRequest(event) {
  let request = event.request

  let url = new URL(request.url);
  if (url.pathname == '/query') {

    let data = await SQLCSV.get('forex.csv')

    let wmod = await emscripten_module
    
    let query = "SELECT count(*) FROM forex";
    if(request.method == "POST") {
        query = await request.text()
    }

    let result = wmod.query(
      "CREATE VIRTUAL TABLE temp.forex USING csv(data='"+data+"',header);"+query,
      String(request.cf.country),
      String(request.cf.asn),
      String(request.cf.colo),
      String(request.cf.city),
      String(request.cf.continent),
      String(request.cf.timezone),
      String(request.cf.latitude),
      String(request.cf.longitude),
      String(request.headers.get("CF-Connecting-IP")),
      String(request.headers.get("User-Agent"))
    )

    let newResponse = new Response(result, {status: 200,
      headers: {
        'Access-Control-Allow-Origin': '*',
        'Access-Control-Allow-Headers': '*'
        }
      })
    return newResponse
  } else if (url.pathname == '/') {
    let page = `<!doctype html>

<html lang="en">
  <head>
    <meta charset="utf-8">

    <title>edge-sql</title>

    <meta property="og:title" content="Edge Worker SQLite" />
    <meta property="og:type" content="website" />
    <meta property="og:url" content="https://sql.lspgn.workers.dev" />
    <meta property="og:image" content="https://raw.githubusercontent.com/lspgn/edge-sql/main/extra/img.png" />
    <meta property="og:description" content="Serverless SQL application" />
    <meta property="description" content="Serverless SQL application" />
    <meta name="twitter:card" content="summary_large_image" />
    <meta name="twitter:creator" content="@lpoinsig" />
    <meta name="twitter:site" content="@lpoinsig" />
  </head>
  <style>
    body{font-family:Helvetica,Arial;margin:20px}#map{height:800px;width:800px;margin:20px}.leaflet-layer-image{image-rendering:crisp-edges}
  </style>
  <body>
    <h1>edge-sql</h1>


    <script type="application/javascript">
      function query() {

      var date1 = new Date();

        var xhr = new XMLHttpRequest();
        xhr.open("POST", 'https://sql.lspgn.workers.dev/query', true);
        xhr.setRequestHeader("Content-Type", "application/text");

        xhr.onreadystatechange = function() {
            if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {

                var date2 = new Date();
                var diff = date2 - date1;

                document.getElementById("duration").innerHTML = diff+' milliseconds'
                document.getElementById("result").innerHTML = xhr.response
            }
        }
        xhr.send(document.getElementById("query").value);

      }
    </script>

    <div>

      <p>A serverless edge worker embedding <a href="https://sqlite.org">SQLite</a> using <a href=https://workers.cloudflare.com/>Cloudflare Workers</a> and WASM.</p>
      <p>Check out the <a href=https://github.com/lspgn/edge-sql>source on GitHub</a> and/or follow me on <a href="https://twitter.com/lpoinsig">Twitter</a>.</p>
      <p>The data used in production is a reduced version (only EUR, JPY, GBP and CHF currencies) of the <a href="https://data.humdata.org/dataset/ecb-fx-rates">European Central Bank Forex Rates on Humdata</a>.
      It is <a href="https://data.humdata.org/about/license" rel="nofollow">distributed</a> under the license <a href="https://creativecommons.org/licenses/by/4.0/legalcode">CC-BY</a>.</p>
      <h3>Example queries</h3>
        <h5>Days GBP was the highest and lowest</h5>
        <pre>SELECT *,1/EUR,1/JPY,1/GBP,1/CHF
FROM forex
WHERE
  GBP = (
    SELECT
      max(GBP)
    FROM forex)
  OR GBP = (
    SELECT
      min(GBP)
    FROM forex)</pre>

        <h5>Use SQLite built-in and extra functions</h5>
        <pre>SELECT
  getdata('country') AS country,
  random() AS rnd,
  date('now') AS now</pre>

    <h3>Try</h3>

      <textarea id="query" rows="20" cols="60">SELECT * FROM forex ORDER BY Date DESC LIMIT 10;</textarea><br>
      <button onclick="query()">Query</button>
    </div>

    <div>
      <pre id="result">
        Result will be printed here
      </pre>
      <p>Duration of the query: <span id="duration">NA</span></p>
    </div>

  </body>
</html>`
    return new Response(page, {status: 200, headers: {"content-type": "text/html"}})
  } else {
    return new Response("not found", {status: 404})
  }
}
