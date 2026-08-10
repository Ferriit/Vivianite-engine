#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
#else
    #include <errno.h>
    #include <fcntl.h>
    #include <poll.h>
    #include <unistd.h>
    #include <sys/wait.h>
#endif

struct Process {
#ifdef _WIN32
    HANDLE process = nullptr;
    HANDLE stdout_read = nullptr;
#else
    pid_t pid = -1;
    int stdout_read = -1;
#endif
};

inline Process process_create(
    const std::string& program,
    const std::vector<std::string>& args = {}
) {
    Process p;

#ifdef _WIN32

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE stdout_write;

    if (!CreatePipe(&p.stdout_read, &stdout_write, &sa, 0))
        throw std::runtime_error("CreatePipe failed");

    // The parent must not give the child a copy of the read end.
    SetHandleInformation(p.stdout_read, HANDLE_FLAG_INHERIT, 0);

    std::string command = "\"" + program + "\"";

    for (const auto& arg : args)
        command += " \"" + arg + "\"";

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = stdout_write;
    si.hStdError = stdout_write;

    PROCESS_INFORMATION pi{};

    if (!CreateProcessA(
        nullptr,
        command.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    )) {
        CloseHandle(p.stdout_read);
        CloseHandle(stdout_write);
        throw std::runtime_error("CreateProcess failed");
    }

    CloseHandle(stdout_write);
    CloseHandle(pi.hThread);

    p.process = pi.hProcess;

#else

    int pipefd[2];

    if (pipe(pipefd) == -1)
        throw std::runtime_error("pipe failed");

    p.pid = fork();

    if (p.pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        throw std::runtime_error("fork failed");
    }

    if (p.pid == 0) {
        // Child
        close(pipefd[0]);

        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        close(pipefd[1]);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(program.c_str()));

        for (const auto& arg : args)
            argv.push_back(const_cast<char*>(arg.c_str()));

        argv.push_back(nullptr);

        execvp(program.c_str(), argv.data());

        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    p.stdout_read = pipefd[0];

    if (fcntl(p.stdout_read, F_SETFL, O_NONBLOCK) == -1) {
        close(p.stdout_read);
        throw std::runtime_error("fcntl failed");
    }

#endif

    return p;
}

inline bool process_is_running(const Process& p) {
#ifdef _WIN32
    return WaitForSingleObject(p.process, 0) == WAIT_TIMEOUT;
#else
    int status = 0;
    pid_t result = waitpid(p.pid, &status, WNOHANG);
    return result == 0;
#endif
}

inline bool process_read_chunk(Process& p, std::string& chunk) {
#ifdef _WIN32
    if (!p.stdout_read)
        return false;

    DWORD bytes_available = 0;
    PeekNamedPipe(p.stdout_read, nullptr, 0, nullptr, &bytes_available, nullptr);

    if (bytes_available == 0)
        return process_is_running(p);

    char buffer[4096];
    DWORD bytes_read = 0;
    if (!ReadFile(p.stdout_read, buffer, std::min<DWORD>(sizeof(buffer), bytes_available), &bytes_read, nullptr))
        return false;

    if (bytes_read > 0) {
        chunk.assign(buffer, bytes_read);
        return true;
    }

    return process_is_running(p);
#else
    if (p.stdout_read < 0)
        return false;

    char buffer[4096];
    pollfd fds{p.stdout_read, POLLIN, 0};
    int ready = poll(&fds, 1, 25);

    if (ready < 0) {
        if (errno == EINTR)
            return true;
        return false;
    }

    if (ready == 0)
        return process_is_running(p);

    ssize_t bytes_read = read(p.stdout_read, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        chunk.assign(buffer, bytes_read);
        return true;
    }

    return false;
#endif
}

inline std::string process_await(Process& p) {
    std::string output;
    char buffer[4096];

#ifdef _WIN32

    DWORD bytes_read;

    while (ReadFile(
        p.stdout_read,
        buffer,
        sizeof(buffer),
        &bytes_read,
        nullptr
    ) && bytes_read > 0) {
        output.append(buffer, bytes_read);
    }

    WaitForSingleObject(p.process, INFINITE);

    CloseHandle(p.stdout_read);
    CloseHandle(p.process);

#else

    ssize_t bytes_read;

    while ((bytes_read = read(
        p.stdout_read,
        buffer,
        sizeof(buffer)
    )) > 0) {
        output.append(buffer, bytes_read);
    }

    close(p.stdout_read);

    waitpid(p.pid, nullptr, 0);

#endif

    return output;
}
