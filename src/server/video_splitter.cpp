/******************************
 *     video_splitter.cpp     *
 *      (フレーム分割器)      *
 ******************************/

#include "video_splitter.hpp"

/* コンストラクタ */
VideoSplitter::VideoSplitter(const char *video_src, int row, int column):
    video(cv::VideoCapture(video_src)),
    row(row),
    column(column)
{
    // 再生する動画のチェック
    if(!this->video.isOpened()){
        std::cerr << "[Error] Video open failed." << std::endl;
        std::exit(EXIT_FAILURE);
    }else{
        // 動画のパラメータを設定
        std::cout << "[Info] Opened video." << video_src << std::endl;
        this->setVideoParams();
    }
}

/* デストラクタ */
VideoSplitter::~VideoSplitter(){}

/* 分割時のパラメータを設定 */
void VideoSplitter::setVideoParams(){
    // 動画ファイルの情報を取得
    const int width = int(this->video.get(CV_CAP_PROP_FRAME_WIDTH)/this->row);
    const int height = int(this->video.get(CV_CAP_PROP_FRAME_HEIGHT)/this->column);
    this->total_frame_num = this->video.get(CV_CAP_PROP_FRAME_COUNT);
    this->fps = this->video.get(CV_CAP_PROP_FPS);
    
    // フレームの分割領域とキューを設定
    const int display_num = this->row * this->column;
    this->region_list = std::vector<cv::Rect>(display_num);
    this->queue_list = std::vector<std::shared_ptr<FrameQueue>>(display_num);
    for(int y=0; y<this->column; ++y){
        for(int x=0; x<this->row; ++x){
           int id = x + this->row * y;
           this->region_list[id] = cv::Rect(x*width, y*height, width, height);
           this->queue_list[id] = std::make_shared<FrameQueue>(32);
        }
    }
    return;
}

/* 分割フレーム用キューのポインタを取得 */
const smt_FrameQueue_t VideoSplitter::getFrameQueuePtr(const int id){
    return this->queue_list[id];
}

/* フレームの分割を開始 */
void VideoSplitter::start(){
    int id;
    for(int i=0; i<this->total_frame_num; ++i){
        // 次番のフレームを分割
        this->video >> this->frame;
        for(int y=0; y<this->column; ++y){
            for(int x=0; x<this->row; ++x){
                id = x + this->row * y;
                this->queue_list[id]->enqueue(cv::Mat(this->frame, this->region_list[id]));
            }
        }
    }
    return;
}
