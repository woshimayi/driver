/*
 * @*************************************:
 * @FilePath     : /network/curl/curl_upload.cpp
 * @version      :
 * @Author       : dof
 * @Date         : 2025-08-06 10:53:01
 * @LastEditors  : dof
 * @LastEditTime : 2025-08-06 10:57:30
 * @Descripttion :
 * @compile      :
 * @**************************************:
 */
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <random>
#include <vector>
#include <curl/curl.h>

using namespace std;
using namespace std::chrono;

struct UploadStats
{
    string start_time;
    string end_time;
    double elapsed_seconds;
    double timeout_seconds;
    size_t data_size_bytes;
    size_t transferred_bytes;
    double upload_speed_kbps;
    string status;
    string target_url;
    vector<pair<double, size_t>> progress_log; // 时间戳和已传输字节
};

// 假数据生成器
class FakeDataGenerator
{
public:
    FakeDataGenerator(size_t total_size) : total_size(total_size), generated(0)
    {
        random_device rd;
        gen.seed(rd());
    }

    size_t generate(char *buffer, size_t chunk_size)
    {
        size_t remaining = total_size - generated;
        size_t to_generate = min(chunk_size, remaining);

        uniform_int_distribution<> dist(0, 255);
        for (size_t i = 0; i < to_generate; ++i)
        {
            buffer[i] = static_cast<char>(dist(gen));
        }

        generated += to_generate;
        return to_generate;
    }

    bool done() const { return generated >= total_size; }

private:
    size_t total_size;
    size_t generated;
    mt19937 gen;
};

// 用于CURL的读取回调函数
static size_t read_callback(char *buffer, size_t size, size_t nitems, void *userdata)
{
    FakeDataGenerator *generator = static_cast<FakeDataGenerator *>(userdata);
    size_t chunk_size = size * nitems;
    size_t generated = generator->generate(buffer, chunk_size);
    return generated;
}

// 用于CURL的进度回调函数
static int progress_callback(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                             curl_off_t ultotal, curl_off_t ulnow)
{
    UploadStats *stats = static_cast<UploadStats *>(clientp);
    auto now = high_resolution_clock::now();
    auto elapsed = duration_cast<duration<double>>(now - stats->start_time_point).count();
    stats->progress_log.emplace_back(elapsed, ulnow);
    return 0;
}

UploadStats upload_fake_data(size_t data_size_bytes,
                             const string &target_url,
                             double timeout_seconds)
{
    UploadStats stats;
    stats.target_url = target_url;
    stats.timeout_seconds = timeout_seconds;
    stats.data_size_bytes = data_size_bytes;

    // 记录开始时间
    stats.start_time_point = high_resolution_clock::now();
    time_t start_time_t = system_clock::to_time_t(system_clock::now());
    tm start_tm = *localtime(&start_time_t);

    stringstream start_ss;
    start_ss << put_time(&start_tm, "%Y-%m-%d %H:%M:%S");
    auto start_micro = duration_cast<microseconds>(stats.start_time_point.time_since_epoch()).count() % 1000000;
    stats.start_time = start_ss.str() + "." + to_string(start_micro);

    CURL *curl = curl_easy_init();
    if (!curl)
    {
        stats.status = "CURL_INIT_ERROR";
        return stats;
    }

    // 设置CURL选项
    FakeDataGenerator generator(data_size_bytes);
    curl_easy_setopt(curl, CURLOPT_URL, target_url.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
    curl_easy_setopt(curl, CURLOPT_READDATA, &generator);
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)data_size_bytes);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, (long)ceil(timeout_seconds));
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &stats);

    // 执行上传
    CURLcode res = curl_easy_perform(curl);

    // 记录结束时间
    auto end = high_resolution_clock::now();
    time_t end_time_t = system_clock::to_time_t(system_clock::now());
    tm end_tm = *localtime(&end_time_t);

    stringstream end_ss;
    end_ss << put_time(&end_tm, "%Y-%m-%d %H:%M:%S");
    auto end_micro = duration_cast<microseconds>(end.time_since_epoch()).count() % 1000000;
    stats.end_time = end_ss.str() + "." + to_string(end_micro);

    // 计算耗时
    stats.elapsed_seconds = duration_cast<duration<double>>(end - stats.start_time_point).count();

    // 获取传输统计信息
    curl_off_t uploaded;
    curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD_T, &uploaded);
    stats.transferred_bytes = static_cast<size_t>(uploaded);

    double speed;
    curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD_T, &speed);
    stats.upload_speed_kbps = speed * 8 / 1024; // 转换为Kbps

    // 确定状态
    if (res == CURLE_OK)
    {
        stats.status = "COMPLETED";
    }
    else if (res == CURLE_OPERATION_TIMEDOUT || stats.elapsed_seconds >= timeout_seconds)
    {
        stats.status = "TIMEOUT";
    }
    else
    {
        stats.status = "FAILED: " + string(curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    return stats;
}

void print_stats_json(const UploadStats &stats)
{
    cout << "{\n";
    cout << "    \"start_time\": \"" << stats.start_time << "\",\n";
    cout << "    \"end_time\": \"" << stats.end_time << "\",\n";
    cout << "    \"elapsed_seconds\": " << fixed << setprecision(6) << stats.elapsed_seconds << ",\n";
    cout << "    \"timeout_seconds\": " << stats.timeout_seconds << ",\n";
    cout << "    \"data_size_bytes\": " << stats.data_size_bytes << ",\n";
    cout << "    \"transferred_bytes\": " << stats.transferred_bytes << ",\n";
    cout << "    \"upload_speed_kbps\": " << stats.upload_speed_kbps << ",\n";
    cout << "    \"status\": \"" << stats.status << "\",\n";
    cout << "    \"target_url\": \"" << stats.target_url << "\",\n";

    cout << "    \"progress_log\": [\n";
    for (size_t i = 0; i < stats.progress_log.size(); ++i)
    {
        cout << "        {\"time\": " << stats.progress_log[i].first
             << ", \"bytes\": " << stats.progress_log[i].second << "}";
        if (i != stats.progress_log.size() - 1)
            cout << ",";
        cout << "\n";
    }
    cout << "    ]\n";
    cout << "}" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        cerr << "Usage: " << argv[0] << " <data_size_bytes> <target_url> <timeout_seconds>" << endl;
        cerr << "Example: " << argv[0] << " 1048576 http://example.com/upload 5.0" << endl;
        return 1;
    }

    size_t data_size = stoul(argv[1]);
    string target_url = argv[2];
    double timeout = stod(argv[3]);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    UploadStats stats = upload_fake_data(data_size, target_url, timeout);
    print_stats_json(stats);
    curl_global_cleanup();

    return 0;
}