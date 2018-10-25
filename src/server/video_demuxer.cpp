/*********************************
 *       video_demuxer.cpp       *
 *    (フレーム分割モジュール)   *
 *********************************/

#include "video_demuxer.hpp"

/* コンストラクタ */
VideoDemuxer::VideoDemuxer(const char *video_src, int row, int column):
    video(cv::VideoCapture(video_src)),
    row(row),
    column(column)
{
    // 再生する動画のチェック
    if(this->video.isOpened() == false){
        std::cerr << "[Error] File open failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    // 動画ファイルの情報を取得
    const int width = int(this->video.get(CV_CAP_PROP_FRAME_WIDTH)/this->row);
    const int height = int(this->video.get(CV_CAP_PROP_FRAME_HEIGHT)/this->column);
    this->total_frame_num = this->video.get(CV_CAP_PROP_FRAME_COUNT);
    this->fps = this->video.get(CV_CAP_PROP_FPS);
    
    // フレームの分割領域を決定
    const int display_num = this->row * this->column;
    this->rects = new cv::Rect[display_num];
    this->div_frames = new cv::Mat[display_num];
    for(int j=0; j<this->column; ++j){
        for(int i=0; i<this->row; ++i){
           this->rects[i+this->row*j] = cv::Rect(i*width, j*height, width, height);
        }
    }
}

/* デストラクタ */
VideoDemuxer::~VideoDemuxer(){}

/* 次番のフレームを分割するメソッド */
void VideoDemuxer::divideNextFrame(){
    // 動画からフレームを取得
    cv::Mat frame;
    this->video >> frame;
    
    // フレームを分割
    int id;
    for(int j=0; j<this->column; ++j){
       for(int i=0; i<this->row; ++i){
           id = i + this->row * j;
           this->div_frames[id] = cv::Mat(frame, this->rects[id]);
       }
    }
    return;
}

/* 分割フレームを取得するメソッド */
cv::Mat *VideoDemuxer::getDividedFrame(int id){
    return &(this->div_frames[id]);
}

