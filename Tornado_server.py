import time
import tornado.auth
import tornado.escape
import tornado.httpserver
import tornado.ioloop
import tornado.options
import tornado.web
from tornado.web import asynchronous
from tornado.options import define, options
from tornado.escape import json_decode
import tornado.concurrent
import detect

define('port', default=11111, help='run on the given port', type=int)
# class HandlePool:
#     def __init__(self, num):
#         self._handles = {}
#         for i in range(num):
#             self._handles[str(detect.get_handle())] = 0
#     def get_handle(self):
#         for k, v in self._handles.iteritems():
#             if v == 0:
#                 self._handles[k] = 1
#                 return int(k)
#         return 0
#     def release_handle(self, handle):
#         handle = str(handle)
#         if handle not in self._handles:
#             return 0
#         self._handles[handle] = 0
#         return 1
# global handler_pool
# handler_pool = HandlePool(5)
# def worker(img_path):
#     st = time.time()
#     img = cv2.imread(img_path)
#     result = faceall_sdk.detect_and_landmark(handle, img)
#     print time.time() - st, result 
handle = detect.get_handle()
class Application(tornado.web.Application):
    def __init__(self):
        handlers = [
        (r"/post", MainHandler),
        ]
        tornado.web.Application.__init__(self, handlers)
        self.executor = tornado.concurrent.futures.ThreadPoolExecutor(4)

class MainHandler(tornado.web.RequestHandler):
    @property
    def executor(self):
        return self.application.executor
    def get_result(self, data):
        result = detect.Detect_landmark(handle, str(data['path']))
        return result

    @asynchronous
    @tornado.gen.coroutine
    def post(self, *args, **kwargs):
        data = json_decode(self.request.body)
        result = yield self.executor.submit(self.get_result, data)
        self.write(str(result))
        self.finish()

def main():
    tornado.options.parse_command_line()
    http_server = tornado.httpserver.HTTPServer(Application())
    http_server.listen(options.port)
    tornado.ioloop.IOLoop.instance().start()

if __name__ == "__main__":
    main()
