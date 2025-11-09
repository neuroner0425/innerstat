#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <string>

#ifndef INNERSTAT_AGENT_SINGLE_INSTANCE_GUARD_H_
#define INNERSTAT_AGENT_SINGLE_INSTANCE_GUARD_H_

// 단일 인스턴스 실행을 위한 가드 클래스
class SingleInstanceGuard {
private:
    std::string lock_path_;
    int fd_ = -1;
    bool acquired_ = false;
public:
    explicit SingleInstanceGuard(const std::string& lock_path) : lock_path_(lock_path) {
        // 잠금 파일 열기 (존재하지 않으면 생성)
        fd_ = ::open(lock_path_.c_str(), O_RDWR | O_CREAT, 0644);
        if (fd_ == -1) {
            std::perror("lock file open failed");
            return;
        }
        // 논블로킹 배타 잠금 시도
        if (flock(fd_, LOCK_EX | LOCK_NB) == -1) {
            // 다른 프로세스가 이미 잠금 보유
            return; 
        }
        acquired_ = true;
        // PID 기록 (정보 목적)
        std::string pid_str = std::to_string(getpid()) + "\n";
        // 기존 내용 덮어쓰기 위해 lseek
        lseek(fd_, 0, SEEK_SET);
        ::write(fd_, pid_str.c_str(), pid_str.size());
        // 파일 길이 잘라내기 (이전 내용 있을 경우)
        ::ftruncate(fd_, pid_str.size());
    }
    bool acquired() const { return acquired_; }
    ~SingleInstanceGuard() {
        if (fd_ != -1) {
            if (acquired_) {
                // flock 해제 (파일 닫히면 자동 해제)
                flock(fd_, LOCK_UN);
            }
            ::close(fd_);
            // 잠금 파일은 유지 (크래시 후 재사용 가능). 필요 시 아래 줄 주석 해제.
            // if (acquired_) ::unlink(lock_path_.c_str());
        }
    }
};

#endif // INNERSTAT_AGENT_SINGLE_INSTANCE_GUARD_H_