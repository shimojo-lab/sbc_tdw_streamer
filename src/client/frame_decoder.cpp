/**************************
*    frame_decoder.cpp    *
*     (フレーム展開器)    *
**************************/

#include "frame_decoder.hpp"

/* コンストラクタ */
FrameDecoder::FrameDecoder(const msgbuf_ptr_t rbuf, const framebuf_ptr_t vbuf, const cv::Rect roi, const int res_x, const int res_y):
    rbuf(rbuf),
    vbuf(vbuf),
    roi(roi),
    res_x(res_x),
    res_y(res_y)
{}

/* フレーム展開を開始 */
void FrameDecoder::run(){
    cv::Mat frame;
    while(true){
        const std::string msg = this->rbuf->pop();
        std::vector<unsigned char> comp_buf(msg.c_str(), msg.c_str()+msg.length());
        this->vbuf->push(cv::Mat(cv::imdecode(comp_buf, CV_LOAD_IMAGE_UNCHANGED), this->roi));
    }
}
