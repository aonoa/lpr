#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "plate_detectors.h"
#include "plate_recognizers.h"
#include "hv/HttpServer.h"

using namespace std;

int main() {

    pr::fix_lffd_detector("models/float", pr::lffd_float_detector);
    pr::PlateDetector detector = pr::IPlateDetector::create_plate_detector(pr::lffd_float_detector);

    pr::fix_lpr_recognizer("models/float", pr::float_lpr_recognizer);
    pr::LPRRecognizer lpr = pr::float_lpr_recognizer.create_recognizer();

    HttpService router;
    router.GET("/ping", [](HttpRequest* req, HttpResponse* resp) {
        return resp->String("pong");
    });

    router.GET("/paths", [&router](HttpRequest* req, HttpResponse* resp) {
        return resp->Json(router.Paths());
    });

    router.POST("/echo", [](HttpRequest* req, HttpResponse* resp) {
        resp->content_type = req->content_type;
        resp->body = req->body;

        return 200;
    });

    router.POST("/echo", [](const HttpContextPtr& ctx) {
        return ctx->send(ctx->body(), ctx->type());
    });

    router.POST("/api/lpr", [&](HttpRequest* req, HttpResponse* resp) {
        auto start = std::chrono::high_resolution_clock::now();
        hv::Json res,resData;
        string fileName = "";
        cv::Mat img;

        if (req->content_type == MULTIPART_FORM_DATA) {
            vector<uchar> data;
            auto body = req->GetFormData("file");
            for (int i = 0; i < body.length(); ++i){
                data.push_back(body[i]);
            }
            img = cv::imdecode(data, -1);
            if (img.empty()) {
                res["code"] = "-1";
                res["message"] = "upload image error";
                return resp->Json(res);
            }

        } else if (req->content_type == APPLICATION_JSON) {
            auto data = req->GetJson();
            try {
                fileName = data["lpr_url"];
            } catch (...) {
                res["code"] = "-1";
                res["message"] = "request param error";
                return resp->Json(res);
            }

            cout << fileName << endl;
            auto cap = cv::VideoCapture(fileName);

            if (!cap.read(img)) {
                res["code"] = "-1";
                res["message"] = "open image error";
                return resp->Json(res);
            }
        }

        ncnn::Mat sample = ncnn::Mat::from_pixels_roi(img.data, ncnn::Mat::PIXEL_BGR, img.cols, img.rows, img.cols/3, img.rows/3, img.cols/2, img.rows/2);
        auto end2 = std::chrono::high_resolution_clock::now();

        std::vector<pr::PlateInfo> objects;
        detector->plate_detect(sample, objects);

        auto end1 = std::chrono::high_resolution_clock::now();
        lpr->decode_plate_infos(objects);

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> tm2 = end2 - start;
        std::cout << "加载耗时: " << tm2.count() << "ms" << std::endl;
        std::chrono::duration<double, std::milli> tm = end1 - end2;
        std::cout << "检测耗时: " << tm.count() << "ms" << std::endl;
        std::chrono::duration<double, std::milli> tm1 = end - end1;
        std::cout << "识别耗时: " << tm1.count() << "ms" << std::endl;

        int num = 0, max = 0;
        string plate_no = "", plate_color = "";

        for (auto pi : objects) {
            // cout << "plate_no: " << pi.plate_color << pi.plate_no << endl;
            if (pi.plate_no.length() > max ) {
                plate_no = pi.plate_no;
                plate_color = pi.plate_color;
                max = pi.plate_no.length();
                num++;
            }
        }

        cout << "plate_no: " << plate_color << plate_no << endl;

        resData["plate_no"] = plate_no;
        resData["plate_color"] = plate_color;

        res["code"] = "200";
        res["data"] = resData;
        return resp->Json(res);
    });

    http_server_t server;
    server.port = 8180;
    server.service = &router;
    server.worker_threads = 1;
    http_server_run(&server);
    return 0;
}

