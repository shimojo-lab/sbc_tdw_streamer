/**************************
*    frame_encoder.cpp    *
*    (フレーム符号化器)   *
**************************/

#include "frame_encoder.hpp"

/* コンストラクタ */
FrameEncoder::FrameEncoder(const std::string video_src, const int column, const int row,
                           const int bezel_w, const int bezel_h, const int width, const int height,
                           jpeg_params_t& sampling_type_list, jpeg_params_t& quality_list,
                           std::vector<tranbuf_ptr_t>& send_bufs):
    handle(tjInitCompress()),
    display_num(column*row),
    sampling_type_list(sampling_type_list),
    quality_list(quality_list),
    send_bufs(send_bufs)
{
    // JPEGエンコーダを初期化
    if(this->handle == NULL){
        std::string err_msg(tjGetErrorStr());
        _ml::caution("Failed to init JPEG encoder", err_msg);
        std::exit(EXIT_FAILURE);
    }
    
    // 表示する動画・画像を読込み
    this->video = cv::VideoCapture(video_src.c_str());
    if(!this->video.isOpened()){
        _ml::caution("Failed to open video", video_src);
        std::exit(EXIT_FAILURE);
    }
    
    // リサイズ用パラメータを設定
    cv::Mat video_frame;
    video >> video_frame;
    this->setResizeParams(
        column, row, bezel_w, bezel_h, width, height, video_frame.cols, video_frame.rows
    );
}

/* デストラクタ */
FrameEncoder::~FrameEncoder(){
    // JPEGエンコーダを破棄
    tjDestroy(this->handle);
}

/* リサイズ用パラメータを設定 */
void FrameEncoder::setResizeParams(const int column, const int row, const int bezel_w,
                                   const int bezel_h, const int width, const int height,
                                   const int frame_w, const int frame_h){
    // リサイズ後のフレームを背景を初期化
    const int bg_w = width * column + bezel_w * 2 * (column - 1);
    const int bg_h = height * row + bezel_h * 2 * (row - 1);
    this->resized_frame = cv::Mat::zeros(cv::Size(bg_w, bg_h), CV_8UC3);
    
    // リサイズ倍率を設定
    const double x_ratio = (double)bg_w / (double)frame_w;
    const double y_ratio = (double)bg_h / (double)frame_h;
    this->ratio = x_ratio<y_ratio ? x_ratio : y_ratio;
    this->interpolation_type = this->ratio>=1 ? cv::INTER_LINEAR : cv::INTER_AREA;
    
    // リサイズフレームのパディングを設定
    const int resize_w = (int)((double)frame_w * this->ratio);
    const int resize_h = (int)((double)frame_h * this->ratio);
    const int paste_x = (int)((double)(bg_w - resize_w) / 2.0);
    const int paste_y = (int)((double)(bg_h - resize_h) / 2.0);
    this->roi = cv::Rect(paste_x, paste_y, resize_w, resize_h);
    
    // ディスプレイノードの担当領域を設定
    this->regions = std::vector<cv::Rect>(this->display_num);
    this->raw_frames = std::vector<cv::Mat>(this->display_num);
    for(int j=0; j<row; ++j){
        for(int i=0; i<column; ++i){
            this->regions[i+column*j] = cv::Rect((width+bezel_w*2)*i,
                                                 (height+bezel_h*2)*j,
                                                 width,
                                                 height);
        }
    }
}

/* フレームをリサイズ */
void FrameEncoder::resize(cv::Mat& video_frame){
    // アスペクト比を維持して拡大
    cv::resize(video_frame,
               video_frame,
               cv::Size(),
               this->ratio,
               this->ratio,
               this->interpolation_type
    );
    
    // 背景と重ねてパディング
    cv::Mat paste_area = this->resized_frame(this->roi);
    video_frame.copyTo(paste_area);
    
    // ディスプレイノードの担当領域毎に分割
    for(int i=0; i<this->display_num; ++i){
        this->resized_frame(this->regions[i]).copyTo(this->raw_frames[i]);
    }
}

/* フレームをJPEGで符号化 */
void FrameEncoder::encode(const int id){
    unsigned char *jpeg_frame = NULL;
    unsigned long jpeg_size = 0;
    const int tj_stat = tjCompress2(this->handle,
                                    this->raw_frames[id].data,
                                    this->raw_frames[id].cols,
                                    this->raw_frames[id].cols*COLOR_CHANNEL_NUM,
                                    this->raw_frames[id].rows,
                                    TJPF_RGB,
                                    &jpeg_frame,
                                    &jpeg_size,
                                    this->sampling_type_list[id].load(std::memory_order_acquire),
                                    this->quality_list[id].load(std::memory_order_acquire),
                                    TJFLAG_FASTDCT
    );
    if(tj_stat == JPEG_FAILED){
        const std::string err_msg(tjGetErrorStr());
        _ml::warn("JPEG encode failed", err_msg);
    }else{
        const std::string jpeg_str(jpeg_frame, jpeg_frame+jpeg_size);
        this->send_bufs[id]->push(jpeg_str);
    }
}

/* フレーム圧縮を開始 (動画表示用) */
void FrameEncoder::run(){
    cv::Mat video_frame;
    while(true){
        // フレームを1枚取り出し
        this->video >> video_frame;
        
        // フレームをリサイズ
        try{
            this->resize(video_frame);
        }catch(...){
            _ml::caution("Could not get video frame", "JPEG encoder stopped");
            break;
        }
        
        // フレームを圧縮
        for(int i=0; i<this->display_num; ++i){
            this->encode(i);
        }
    }
}

