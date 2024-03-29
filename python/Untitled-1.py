from __future__ import print_function
from builtins import bytes

try:
    from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
except ImportError:
    from http.server import BaseHTTPRequestHandler, HTTPServer

from json import loads

try:
    from urllib2 import urlopen, HTTPError, URLError
except ImportError:
    from urllib.request import urlopen
    from urllib.error import HTTPError, URLError

from sys import stderr, argv, exit

ver='1.1'

def postDomo(srv, idx, val):
    url = "http://{}/json.htm?type=command&param=udevice&idx={}&nvalue=0&svalue={}".format(srv, idx, val)
    rc = False;

    try:
        page = urlopen(url)
        rc = True
    except HTTPError as err:
        msg = "Domoticz update error: url={}, reason={}, page={}".format(err.geturl(), err.reason, err.read())
        print(msg, file=stderr)
    except URLError as err:
        msg = "Domoticz update error: url={}, reason={}".format(url, err.reason)
        print(msg, file=stderr)

    return rc

class Gateway(BaseHTTPRequestHandler):
    def respond( self, code, msg ):
        self.send_response(code)
        self.send_header("Content-Type", "text/ascii")
        self.send_header("Content-Length", len(bytes(msg, 'utf8')))
        self.end_headers()
        self.wfile.write(bytes(msg, 'utf8'))

    def do_POST( self ):
        content_length = int(self.headers.get("Content-Length", 0))
        json_data = loads(self.rfile.read(content_length).decode('utf-8'))

        foundValues = updatedValues = 0
        for value in json_data["sensordatavalues"]:
            if value["value_type"] in self.server.mappings:
                foundValues += 1
                if postDomo(self.server.domo, self.server.mappings[value["value_type"]], value["value"]):
                    updatedValues += 1
        if foundValues == len(self.server.mappings):
            if updatedValues == len(self.server.mappings):
                self.respond(200, "OK")
            else:
                self.respond(502, "ERROR") # domoticz server error
        else:
            self.respond(400, "ERROR") # airrohr json incomplete

if __name__ == "__main__":
    print("{} version {}".format(argv[0], ver))

    try:
        mappings = dict(pair.split('=', 1) for pair in argv[3:])
    except ValueError as err:
        mappings = {}
        print("Mapping {}: {}".format(argv[3:], err), file=stderr)
    print(256)
    if len(mappings) == 0:
        usage = "Usage: ./airrohr2domoticz.py listen_port domoticz_server[:port] airrohr_value_type=domoticz_index..."
        print(usage, file=stderr)
        exit(1)

    httpd = HTTPServer(("", int(argv[1])), Gateway)
    httpd.domo = argv[2]
    httpd.mappings = mappings
    
    print("Starting gateway from port {} to {} for mapping {}".format(argv[1], httpd.domo, mappings))
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print(" interrupted.")

    httpd.server_close()
    print("Gateway stopped.")