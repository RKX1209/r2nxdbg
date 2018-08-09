/* r2nxdbg agent - MIT - Copyright 2018 - rkx1209(rkx1209dev@gmail.com) */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <switch.h>

int listen_fd, client_fd;
typedef enum {
    REQ_LIST_PROCESSES   =0,
    REQ_ATTACH_PROCESS   =1,
    REQ_DETACH_PROCESS   =2,
    REQ_QUERYMEMORY      =3,
    REQ_GET_DBGEVENT     =4,
    REQ_READMEMORY       =5,
    REQ_CONTINUE_DBGEVENT=6,
    REQ_GET_THREADCONTEXT=7,
    REQ_BREAK_PROCESS    =8,
    REQ_WRITEMEMORY32    =9,
    REQ_LISTENAPPLAUNCH  =10,
    REQ_GETAPPPID        =11,
    REQ_START_PROCESS    =12,
    REQ_GET_TITLE_PID    =13
} RequestType;

typedef struct {
    u32 Type;
} DebuggerRequest;

typedef struct {
    u32 Result;
    u32 LenBytes;
    void* Data;
} DebuggerResponse;


typedef struct { // Cmd1
    u64 Pid;
} AttachProcessReq;

typedef struct {
    u32 DbgHandle;
} AttachProcessResp;

typedef struct { // Cmd2
    u32 DbgHandle;
} DetachProcessReq;

typedef struct { // Cmd3
    u32 DbgHandle;
    u32 Pad;
    u64 Addr;
} QueryMemoryReq;

typedef struct {
    u64 Addr;
    u64 Size;
    u32 Perm;
    u32 Type;
} QueryMemoryResp;

typedef struct { // Cmd4
    u32 DbgHandle;
} GetDbgEventReq;

typedef struct {
    u8 Event[0x40];
} GetDbgEventResp;

typedef struct { // Cmd5
    u32 DbgHandle;
    u32 Size;
    u64 Addr;
} ReadMemoryReq;

typedef struct { // Cmd6
    u32 DbgHandle;
    u32 Flags;
    u64 ThreadId;
} ContinueDbgEventReq;

typedef struct { // Cmd7
    u32 DbgHandle;
    u32 Flags;
    u64 ThreadId;
} GetThreadContextReq;

typedef struct {
    u8 Out[0x320];
} GetThreadContextResp;

typedef struct { // Cmd8
    u32 DbgHandle;
} BreakProcessReq;

typedef struct { // Cmd9
    u32 DbgHandle;
    u32 Value;
    u64 Addr;
} WriteMemory32Req;

typedef struct { // Cmd11
    u64 Pid;
} GetAppPidResp;

typedef struct { // Cmd12
    u64 Pid;
} StartProcessReq;

typedef struct { // Cmd13
    u64 TitleId;
} GetTitlePidReq;

typedef struct {
    u64 Pid;
} GetTitlePidResp;


void sendResponse(DebuggerResponse resp) {
    send(client_fd, (void*)&resp, 8, 0);

    if (resp.LenBytes > 0)
        send(client_fd, resp.Data, resp.LenBytes, 0);
}

int handleCommand() {
    DebuggerRequest req;
    DebuggerResponse resp;
    Result rc;
    ssize_t num_bytes;

    printf("Handle Command\n");

    if((num_bytes = read(client_fd, &req, sizeof(req))) < 0) {
	printf("failed to recv");
        return 1;
    }

    resp.LenBytes = 0;
    resp.Data = NULL;

    printf("Read: %lu Req: %d", num_bytes, req.Type);

    switch (req.Type) {
    case REQ_LIST_PROCESSES: { // Cmd0
        static u64 pids[256];
        u32 numOut = 256;

        rc = svcGetProcessList(&numOut, pids, numOut);
        resp.Result = rc;

        if (rc == 0) {
            resp.LenBytes = numOut * sizeof(u64);
            resp.Data = &pids[0];
        }

        sendResponse(resp);
        break;
    }
    case REQ_ATTACH_PROCESS: { // Cmd1
        AttachProcessReq   req_;
        AttachProcessResp  resp_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = svcDebugActiveProcess(&resp_.DbgHandle, req_.Pid);
        resp.Result = rc;

        if (rc == 0) {
            resp.LenBytes = sizeof(resp_);
            resp.Data = &resp_;
        }

        sendResponse(resp);
        break;
    }

    case REQ_DETACH_PROCESS: { // Cmd2
        DetachProcessReq req_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = svcCloseHandle(req_.DbgHandle);
        resp.Result = rc;

        sendResponse(resp);
        break;
    }

    case REQ_QUERYMEMORY: { // Cmd3
        QueryMemoryReq   req_;
        QueryMemoryResp  resp_;
        recv(client_fd, &req_, sizeof(req_), 0);

        MemoryInfo info;
        u32 who_cares;
        rc = svcQueryDebugProcessMemory(&info, &who_cares, req_.DbgHandle, req_.Addr);
        resp.Result = rc;

        if (rc == 0) {
            resp_.Addr = info.addr;
            resp_.Size = info.size;
            resp_.Type = info.type;
            resp_.Perm = info.perm;

            resp.LenBytes = sizeof(resp_);
            resp.Data = &resp_;
        }

        sendResponse(resp);
        break;
    }

    case REQ_GET_DBGEVENT: { // Cmd4
        GetDbgEventReq   req_;
        GetDbgEventResp  resp_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = svcGetDebugEvent(&resp_.Event[0], req_.DbgHandle);
        resp.Result = rc;

        if (rc == 0) {
            resp.LenBytes = sizeof(resp_);
            resp.Data = &resp_;
        }

        sendResponse(resp);
        break;
    }

    case REQ_READMEMORY: { // Cmd5
        ReadMemoryReq req_;
        recv(client_fd, &req_, sizeof(req_), 0);

        if (req_.Size > 0x1000)
            // Too big read.
            fatalSimple(222 | (5 << 9));

        static u8 page[0x1000];
        rc = svcReadDebugProcessMemory(page, req_.DbgHandle, req_.Addr, req_.Size);
        resp.Result = rc;

        if (rc == 0) {
            resp.LenBytes = req_.Size;
            resp.Data = &page[0];
        }

        sendResponse(resp);
        break;
    }

    case REQ_CONTINUE_DBGEVENT: { // Cmd6
        ContinueDbgEventReq req_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = svcContinueDebugEvent(req_.DbgHandle, req_.Flags, req_.ThreadId);
        resp.Result = rc;

        sendResponse(resp);
        break;
    }

    case REQ_GET_THREADCONTEXT: { // Cmd7
        GetThreadContextReq   req_;
        GetThreadContextResp  resp_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = svcGetDebugThreadContext(&resp_.Out[0], req_.DbgHandle, req_.ThreadId, req_.Flags);
        resp.Result = rc;

        if (rc == 0) {
            resp.LenBytes = sizeof(resp_);
            resp.Data = &resp_;
        }

        sendResponse(resp);
        break;
    }

    case REQ_BREAK_PROCESS: { // Cmd8
        BreakProcessReq req_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = svcBreakDebugProcess(req_.DbgHandle);
        resp.Result = rc;

        sendResponse(resp);
        break;
    }

    case REQ_WRITEMEMORY32: { // Cmd9
        WriteMemory32Req req_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = svcWriteDebugProcessMemory(req_.DbgHandle, (void*)&req_.Value, req_.Addr, 4);
        resp.Result = rc;

        sendResponse(resp);
        break;
    }

    case REQ_LISTENAPPLAUNCH: { // Cmd10
        Handle h;
        rc = pmdmntEnableDebugForApplication(&h);
        resp.Result = rc;

        if (rc == 0)
            svcCloseHandle(h);

        sendResponse(resp);
        break;
    }

    case REQ_GETAPPPID: { // Cmd11
        GetAppPidResp resp_;

        rc = pmdmntGetApplicationPid(&resp_.Pid);
        resp.Result = rc;

        if (rc == 0) {
            resp.LenBytes = sizeof(resp_);
            resp.Data = &resp_;
        }

        sendResponse(resp);
        break;
    }

    case REQ_START_PROCESS: { // Cmd12
        StartProcessReq req_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = pmdmntStartProcess(req_.Pid);
        resp.Result = rc;

        sendResponse(resp);
        break;
    }

    case REQ_GET_TITLE_PID: { // Cmd13
        GetTitlePidReq   req_;
        GetTitlePidResp  resp_;
        recv(client_fd, &req_, sizeof(req_), 0);

        rc = pmdmntGetTitlePid(&resp_.Pid, req_.TitleId);
        resp.Result = rc;

        if (rc == 0) {
            resp.LenBytes = sizeof(resp_);
            resp.Data = &resp_;
        }

        sendResponse(resp);
        break;
    }

    default:
        // Unknown request.
        printf("Unknown request %d\n", req.Type);
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
        Result rc;
        char bind_ip_addr[4] = {0, 0, 0, 0};

	struct sockaddr_in bind_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(4444),
		.sin_addr = {
			.s_addr = *(uint32_t*) bind_ip_addr
		}
	};
        struct sockaddr_in remote_addr;
        socklen_t remote_addr_len = sizeof(remote_addr);

        gfxInitDefault();
        consoleInit(NULL);

        printf(CONSOLE_ESC(2J));

        printf("Launch Nxdbg agent\n");

        rc = pmdmntInitialize();
        printf("pmdmntInit = %d\n", rc);
        if (rc) {
                //Failed to get PM debug interface.
                printf(CONSOLE_ESC(28D)"Failed PM debug interface\n");
                fatalSimple(222 | (6 << 9));
        }
        rc = socketInitializeDefault();
        printf("socketInit = %d\n", rc);

        if (rc) {
                printf("failed socket init\n");
                goto fatal;
        }

        if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
                printf("failed socket\n");
                goto fatal;
        }

        if(bind(listen_fd, (struct sockaddr*) &bind_addr, sizeof(bind_addr)) < 0) {
		printf("failed to bind socket\n");
		goto error;
	}

	if(listen(listen_fd, 20) != 0) {
		printf("failed to listen on socket\n");
		goto error;
	}

        printf("Agent init\n");
        while(appletMainLoop())
        {
                //Scan all the inputs. This should be done once for each frame
                hidScanInput();

                // Your code goes here

                //hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
                u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

                if (kDown & KEY_PLUS) break; // break in order to return to hbmenu

	        if((client_fd = accept(listen_fd, (struct sockaddr*) &remote_addr, &remote_addr_len)) < 0) {
		       printf("failed to accept\n");
		       goto error;
	        }

                handleCommand();

                gfxFlushBuffers();
                gfxSwapBuffers();
                gfxWaitForVsync();
                close (client_fd);
        }
error:
        close (listen_fd);
fatal:
        gfxExit();
        return 0;
}
