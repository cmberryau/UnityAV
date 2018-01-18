#include "stdafx.h"
#include "Player.h"
#include "AVLibPlayer.h"

namespace UnityAV
{
    namespace Media
    {
        const string Player::RTSPPrefix = "rtsp://";
        const string Player::FilePrefix = "file://";
            
        Player::Player(const string& uri, unique_ptr<IVideoClient> client) : _uri(uri)
        {
            _videoClient = move(client);         
        }

        unique_ptr<Player> Player::Create(const string& uri, unique_ptr<IVideoClient> client)
        {
            // if we're looking at a file, not an rtsp stream
            if (uri.find(RTSPPrefix) == string::npos)
            {
                ifstream stream(uri);

                if (!stream.good())
                {
                    Debug::LogError("File does not exist at given uri: %s", uri);
                    return nullptr;
                }
            }

            return make_unique<AVLibPlayer>(uri, move(client));
        }

        void Player::Write()
        {
            _videoClient->Write();
        }

        void Player::Visit(VideoFrame& frame)
        {
            OnDisplayVideoFrame(frame);
        }

        void Player::OnFrameReady(Frame& frame)
        {
            // visit the frame
            frame.Accept(*this);
        }

        const IVideoDescription& Player::RequiredVideoFrame() const
        {
            return *_videoClient;
        }

        void Player::OnDisplayVideoFrame(VideoFrame& frame)
        {
            _videoClient->OnFrameReady(frame);
        }
    }
}
