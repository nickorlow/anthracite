# error_gen.py 
# Generates default html error pages for Anthracite

import os

version = "0.1.5"

def generate_error_page(error_code, error_title, error_description):
    html = f"""<html>
  <head><title>{error_title}</title></head>
  <body>
    <center>
      <h1>{error_code} - {error_title}</h1>
      <hr>
      <p>{error_description}</p>
      <p>Anthracite/{version}</p>
      <p><small><a href="https://github.com/nickorlow/anthracite">This is Open Source Software</small></a></p>
    </center>
  </body>
</html>"""
    return html

error_codes = {
    400: "Bad Request",
    401: "Unauthorized",
    402: "Payment Required",
    403: "Forbidden",
    404: "Not Found",
    405: "Method Not Allowed",
    406: "Not Acceptable",
    407: "Proxy Authentication Required",
    408: "Request Timeout",
    409: "Conflict",
    410: "Gone",
    411: "Length Required",
    412: "Precondition Failed",
    413: "Payload Too Large",
    414: "URI Too Long",
    415: "Unsupported Media Type",
    416: "Range Not Satisfiable",
    417: "Expectation Failed",
    418: "I'm a teapot",
    421: "Misdirected Request",
    422: "Unprocessable Entity",
    423: "Locked",
    424: "Failed Dependency",
    425: "Too Early",
    426: "Upgrade Required",
    428: "Precondition Required",
    429: "Too Many Requests",
    431: "Request Header Fields Too Large",
    451: "Unavailable For Legal Reasons",
    500: "Internal Server Error",
    501: "Not Implemented",
    502: "Bad Gateway",
    503: "Service Unavailable",
    504: "Gateway Timeout",
    505: "HTTP Version Not Supported",
    506: "Variant Also Negotiates",
    507: "Insufficient Storage",
    508: "Loop Detected",
    510: "Not Extended",
    511: "Network Authentication Required"
}


error_dir = './error_pages'
os.makedirs(error_dir, exist_ok=True)

for code, title in error_codes.items():
    error_description = error_codes[code]
    error_page = generate_error_page(code, title, error_description)
    file_path = os.path.join(error_dir, f"{code}.html")
    with open(file_path, "w") as file:
        file.write(error_page)