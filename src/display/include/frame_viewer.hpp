/*************************
*    frame_viewer.hpp    *
*    (フレーム表示器)    *
*************************/

#ifndef FRAME_VIEWER_HPP
#define FRAME_VIEWER_HPP

#include "mutex_logger.hpp"
#include "transceive_framebuffer.hpp"
#include "socket_utils.hpp"
#include "sync_message_generator.hpp"
#include <cstring>
extern "C"{
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/mman.h>
    #include <linux/fb.h>
    #include <linux/kd.h>
}

const int DEVICE_OPEN_FAILED = -1;                         // デバイスファイルオープン失敗時の返り値
const int BITS_PER_PIXEL = 24;                             // 1ピクセルあたりのビット数
constexpr double MAX_FPS = 60.0;                           // 最大フレームレート
constexpr int DISPLAY_INTERVAL = (int)(1000.0 / MAX_FPS);  // フレーム表示後の待機時間

/* フレーム表示器 */
class FrameViewer{
    private:
        _asio::io_service& ios;           // I/Oイベントループ
        _ip::tcp::socket& sock;           // TCPソケット
        _asio::streambuf stream_buf;      // ストリームバッファ
        const viewbuf_ptr_t view_buf;     // 表示フレームバッファ
        int fb;                           // フレームバッファのデバイスファイル
        int tty;                          // デバイス端末のデバイスファイル
        int fb_size;                      // フレームバッファのサイズ
        unsigned char *fb_ptr;            // フレームバッファの先頭
        SyncMessageGenerator& generator;  // 同期メッセージ生成器
        const unsigned char *next_frame;  // 次番フレーム
        hr_chrono_t pre_t;                // 処理開始時刻
        hr_chrono_t post_t;               // 処理終了時刻
        hr_chrono_t view_start_t;         // 表示1周期の開始時刻
        hr_chrono_t view_end_t;           // 表示1周期の終了時刻
        
        const bool openFramebuffer(const std::string& fb_dev,  // フレームバッファをオープン
                                   const int width, const int height);
        void hideCursor(const std::string& tty_dev);           // カーソルを非表示化
        void displayFrame();                                   // フレームを表示
        void sendSync();                                       // 同期メッセージを送信
        void onRecvSync(const err_t& err, size_t t_bytes);     // 同期メッセージ受信時のコールバック
        void onSendSync(const err_t& err, size_t t_bytes);     // 同期メッセージ送信時のコールバック
    
    public:
        FrameViewer(_asio::io_service& ios, _ip::tcp::socket& sock,  // コンストラクタ
                    const viewbuf_ptr_t view_buf, const std::string& fb_dev,
                    const int width, const int height, const std::string& tty_dev,
                    SyncMessageGenerator& generator);
        ~FrameViewer();  // デストラクタ
};

#endif  /* FRAME_VIEWER_HPP */

