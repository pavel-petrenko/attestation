#include <iostream>
#include <cstdlib>
#include <mbedtls/x509_crt.h>
#include <Windows.h>
#include "MyTimer.h"

const int MaxThreads = 7;
const int SecondsPerTestPass = 3;
const mbedtls_x509_crt_profile mbedtls_x509_crt_profile_test =
{
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA224) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA256) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA384) |
    MBEDTLS_X509_ID_FLAG(MBEDTLS_MD_SHA512),
    0xFFFFFFF,
    0xFFFFFFF,
    1024,
};

struct ThreadData
{
    int threadIndex;
    bool stopNow;
    int totalCount;
    int successCount;

    ThreadData() : threadIndex(0), stopNow(false), totalCount(0), successCount(0) {}
};

DWORD WINAPI mbedTlsTestMethod(LPVOID lpParam);

int main()
{
    std::cout << std::endl;
#ifdef _DEBUG
    std::cout << "Standalone Debug Test" << std::endl;
#else
    std::cout << "Standalone Release Test" << std::endl;
#endif
    std::cout << "Thread Count, Total Time, Total Count, Total RPS, RPS Per Thread" << std::endl;

    for (int i = 1; i <= MaxThreads; i++)
    {
        ThreadData *apData = new ThreadData[i];
        DWORD      *adwThreadId = (DWORD *) calloc(i, sizeof(DWORD));
        HANDLE     *ahThread = (HANDLE *) calloc(i, sizeof(HANDLE));
        Timer       myTimer;

        for (int j = 0; j < i; j++)
        {
            apData[j].threadIndex = j;

            ahThread[j] = CreateThread(NULL,
                0,
                mbedTlsTestMethod,
                &apData[j],
                0,
                &adwThreadId[j]);
        }

        while (myTimer.elapsed() < SecondsPerTestPass) {
            Sleep(1000);
        }
        for (int j = 0; j < i; j++) {
            apData[j].stopNow = true;
        }
        WaitForMultipleObjects(i, ahThread, TRUE, INFINITE);

        double totalSeconds = myTimer.elapsed();
        int totalSuccess = 0;
        int totalCalls = 0;
        for (int j = 0; j < i; j++) {
            totalCalls += apData[j].totalCount;
            totalSuccess += apData[j].successCount;
        }
        
        std::cout << i << ", " << totalSeconds << ", " << totalSuccess << ", " << totalSuccess / totalSeconds << ", " << totalSuccess / (totalSeconds * i) << std::endl;

        for (int j = 0; j < i; j++) {
            CloseHandle(ahThread[j]);
        }
        delete apData;
        free(adwThreadId);
        free(ahThread);
    }
}

DWORD WINAPI mbedTlsTestMethod(LPVOID lpParam) {
    ThreadData *pData = (ThreadData *)lpParam;

    while (!pData->stopNow)
    {
        pData->totalCount++;

        uint32_t verifyFlags = 0;
        mbedtls_x509_crt crt;
        mbedtls_x509_crt_init(&crt);

        unsigned char* pCertChainBuffer = (unsigned char *) "-----BEGIN CERTIFICATE-----\nMIIEgDCCBCagAwIBAgIUUWVEQfGIDHg6E/gxjpQGNIHQvvQwCgYIKoZIzj0EAwIw\ncTEjMCEGA1UEAwwaSW50ZWwgU0dYIFBDSyBQcm9jZXNzb3IgQ0ExGjAYBgNVBAoM\nEUludGVsIENvcnBvcmF0aW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkGA1UE\nCAwCQ0ExCzAJBgNVBAYTAlVTMB4XDTIwMDUyMjA3MTA1MFoXDTI3MDUyMjA3MTA1\nMFowcDEiMCAGA1UEAwwZSW50ZWwgU0dYIFBDSyBDZXJ0aWZpY2F0ZTEaMBgGA1UE\nCgwRSW50ZWwgQ29ycG9yYXRpb24xFDASBgNVBAcMC1NhbnRhIENsYXJhMQswCQYD\nVQQIDAJDQTELMAkGA1UEBhMCVVMwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAASB\n4r8wLt/UArk1WIjpQCu0Wr4LRsdzbsVkQ+igoHTo+xaBCQoyhP/CUJbNaqZG0Xkl\n8A+mpHAQk4eGYi288fNFo4ICmzCCApcwHwYDVR0jBBgwFoAU0Oiq2nXX+S5JF5g8\nexRl0NXyWU0wXwYDVR0fBFgwVjBUoFKgUIZOaHR0cHM6Ly9hcGkudHJ1c3RlZHNl\ncnZpY2VzLmludGVsLmNvbS9zZ3gvY2VydGlmaWNhdGlvbi92Mi9wY2tjcmw/Y2E9\ncHJvY2Vzc29yMB0GA1UdDgQWBBQr/rqGz890jStzvmfTSksScA7t5jAOBgNVHQ8B\nAf8EBAMCBsAwDAYDVR0TAQH/BAIwADCCAdQGCSqGSIb4TQENAQSCAcUwggHBMB4G\nCiqGSIb4TQENAQEEEL3R6z7z4g4t/7ehb1PAfiMwggFkBgoqhkiG+E0BDQECMIIB\nVDAQBgsqhkiG+E0BDQECAQIBDjAQBgsqhkiG+E0BDQECAgIBDjAQBgsqhkiG+E0B\nDQECAwIBAjAQBgsqhkiG+E0BDQECBAIBBDAQBgsqhkiG+E0BDQECBQIBATARBgsq\nhkiG+E0BDQECBgICAIAwEAYLKoZIhvhNAQ0BAgcCAQYwEAYLKoZIhvhNAQ0BAggC\nAQAwEAYLKoZIhvhNAQ0BAgkCAQAwEAYLKoZIhvhNAQ0BAgoCAQAwEAYLKoZIhvhN\nAQ0BAgsCAQAwEAYLKoZIhvhNAQ0BAgwCAQAwEAYLKoZIhvhNAQ0BAg0CAQAwEAYL\nKoZIhvhNAQ0BAg4CAQAwEAYLKoZIhvhNAQ0BAg8CAQAwEAYLKoZIhvhNAQ0BAhAC\nAQAwEAYLKoZIhvhNAQ0BAhECAQowHwYLKoZIhvhNAQ0BAhIEEA4OAgQBgAYAAAAA\nAAAAAAAwEAYKKoZIhvhNAQ0BAwQCAAAwFAYKKoZIhvhNAQ0BBAQGAJBu1QAAMA8G\nCiqGSIb4TQENAQUKAQAwCgYIKoZIzj0EAwIDSAAwRQIgPrb/pKORoe15Ok/oR7hO\n0yRbmxN4SI8KorjMZhQZDT8CIQCTvHlUQT+ttqt7XnYVVM1YMTXV+eW9kdJvmgBS\nBS0wIw==\n-----END CERTIFICATE-----\n-----BEGIN CERTIFICATE-----\nMIIClzCCAj6gAwIBAgIVANDoqtp11/kuSReYPHsUZdDV8llNMAoGCCqGSM49BAMC\nMGgxGjAYBgNVBAMMEUludGVsIFNHWCBSb290IENBMRowGAYDVQQKDBFJbnRlbCBD\nb3Jwb3JhdGlvbjEUMBIGA1UEBwwLU2FudGEgQ2xhcmExCzAJBgNVBAgMAkNBMQsw\nCQYDVQQGEwJVUzAeFw0xODA1MjExMDQ1MDhaFw0zMzA1MjExMDQ1MDhaMHExIzAh\nBgNVBAMMGkludGVsIFNHWCBQQ0sgUHJvY2Vzc29yIENBMRowGAYDVQQKDBFJbnRl\nbCBDb3Jwb3JhdGlvbjEUMBIGA1UEBwwLU2FudGEgQ2xhcmExCzAJBgNVBAgMAkNB\nMQswCQYDVQQGEwJVUzBZMBMGByqGSM49AgEGCCqGSM49AwEHA0IABL9q+NMp2IOg\ntdl1bk/uWZ5+TGQm8aCi8z78fs+fKCQ3d+uDzXnVTAT2ZhDCifyIuJwvN3wNBp9i\nHBSSMJMJrBOjgbswgbgwHwYDVR0jBBgwFoAUImUM1lqdNInzg7SVUr9QGzknBqww\nUgYDVR0fBEswSTBHoEWgQ4ZBaHR0cHM6Ly9jZXJ0aWZpY2F0ZXMudHJ1c3RlZHNl\ncnZpY2VzLmludGVsLmNvbS9JbnRlbFNHWFJvb3RDQS5jcmwwHQYDVR0OBBYEFNDo\nqtp11/kuSReYPHsUZdDV8llNMA4GA1UdDwEB/wQEAwIBBjASBgNVHRMBAf8ECDAG\nAQH/AgEAMAoGCCqGSM49BAMCA0cAMEQCIC/9j+84T+HztVO/sOQBWJbSd+/2uexK\n4+aA0jcFBLcpAiA3dhMrF5cD52t6FqMvAIpj8XdGmy2beeljLJK+pzpcRA==\n-----END CERTIFICATE-----\n-----BEGIN CERTIFICATE-----\nMIICjjCCAjSgAwIBAgIUImUM1lqdNInzg7SVUr9QGzknBqwwCgYIKoZIzj0EAwIw\naDEaMBgGA1UEAwwRSW50ZWwgU0dYIFJvb3QgQ0ExGjAYBgNVBAoMEUludGVsIENv\ncnBvcmF0aW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkGA1UECAwCQ0ExCzAJ\nBgNVBAYTAlVTMB4XDTE4MDUyMTEwNDExMVoXDTMzMDUyMTEwNDExMFowaDEaMBgG\nA1UEAwwRSW50ZWwgU0dYIFJvb3QgQ0ExGjAYBgNVBAoMEUludGVsIENvcnBvcmF0\naW9uMRQwEgYDVQQHDAtTYW50YSBDbGFyYTELMAkGA1UECAwCQ0ExCzAJBgNVBAYT\nAlVTMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEC6nEwMDIYZOj/iPWsCzaEKi7\n1OiOSLRFhWGjbnBVJfVnkY4u3IjkDYYL0MxO4mqsyYjlBalTVYxFP2sJBK5zlKOB\nuzCBuDAfBgNVHSMEGDAWgBQiZQzWWp00ifODtJVSv1AbOScGrDBSBgNVHR8ESzBJ\nMEegRaBDhkFodHRwczovL2NlcnRpZmljYXRlcy50cnVzdGVkc2VydmljZXMuaW50\nZWwuY29tL0ludGVsU0dYUm9vdENBLmNybDAdBgNVHQ4EFgQUImUM1lqdNInzg7SV\nUr9QGzknBqwwDgYDVR0PAQH/BAQDAgEGMBIGA1UdEwEB/wQIMAYBAf8CAQEwCgYI\nKoZIzj0EAwIDSAAwRQIgQQs/08rycdPauCFk8UPQXCMAlsloBe7NwaQGTcdpa0EC\nIQCUt8SGvxKmjpcM/z0WP9Dvo8h2k5du1iWDdBkAn+0iiA==\n-----END CERTIFICATE-----\n";
        size_t certChainBufferLength = strlen((const char *) pCertChainBuffer) + 1;

        int ret1 = mbedtls_x509_crt_parse(&crt, pCertChainBuffer, certChainBufferLength);
        if (ret1 != 0) {
            printf("mbedtls_x509_crt_parse returned %d\n", ret1);
        }

        int ret2 = mbedtls_x509_crt_verify_with_profile(&crt, crt.next, 0, &mbedtls_x509_crt_profile_test, 0, &verifyFlags, 0, 0);
        if (ret2 != 0) {
            printf("mbedtls_x509_crt_verify_with_profile returned %d\n", ret2);
        }

        if ((ret1 == 0) && (ret2 == 0)) {
            pData->successCount++;
        }
    }

    return pData->successCount;
}
    