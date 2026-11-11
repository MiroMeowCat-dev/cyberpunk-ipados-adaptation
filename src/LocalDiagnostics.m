#import <Foundation/Foundation.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
__attribute__((constructor)) static void CaptureLocalConsole(void) {
    @autoreleasepool {
        if (![NSBundle.mainBundle.infoDictionary[@"SHIMCaptureLocalConsole"] boolValue]) return;
        NSString *dir = [NSHomeDirectory() stringByAppendingPathComponent:@"Documents/PortDiagnostics"];
        if (![[NSFileManager defaultManager] createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil]) return;
        NSString *path = [dir stringByAppendingPathComponent:@"startup-console.txt"];
        int fd = open(path.fileSystemRepresentation, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        if (fd < 0) return;
        dup2(fd, STDOUT_FILENO); dup2(fd, STDERR_FILENO); close(fd);
        setvbuf(stdout, NULL, _IONBF, 0); setvbuf(stderr, NULL, _IONBF, 0);
        fprintf(stderr, "PORTDIAG local capture active; original asset and authorization behavior retained\n");
    }
}
