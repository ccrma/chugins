#ifndef DbImgSampler_h
#define DbImgSampler_h
/**
 *
 * Copyright (c) 2022 Dana Batali
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "../lib/concurrentQ.h"
#include "../lib/image.h"

#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

/**
 * @brief DbImgSampler samples image files given a normalized image coordinate 
 * pair. DbImgSampler reads image files (.png) representing arbitrary parameter
 * values for application in image synthesis or processing.  
 *
 * Images are loaded in a separate thread to prevent hiccups in the
 * audio/realtime thread.  Since the image load is asynchronous, sample
 * values are only valid after the load has completed succesfully.
 */
 
class DbImgSampler
{
public:
    DbImgSampler();
    ~DbImgSampler();

    void Load(char const *filename); // asynchronous
    int GetSample(float x, float y, int *result);

private:
    std::string m_imgfilePath;

    static void loadThreadFunc(DbImgSampler *);

    std::thread::id m_mainThreadId, m_loadThreadId;
    std::thread m_loadThread;
    ConcurrentQ<std::function<void()>> m_loadQueue;

private:
    using t_Instant = std::chrono::time_point<std::chrono::steady_clock>;
    using t_Duration = std::chrono::duration<unsigned int, std::nano>;
    t_Instant m_scheduledUpdate;
    t_Instant m_zeroInstant;
    void makeDirty();
    void checkDeferredUpdate();

private: // image loading
    void loadImage(std::string filename); // in loadThread
    Image *getImage();
    void setImage(Image *, std::string &nm);

    std::mutex m_loadImageLock;
    Image *m_image;
    int m_verbosity;
};

#endif