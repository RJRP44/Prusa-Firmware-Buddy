#include "headers.h"

#include <cassert>
#include <cinttypes>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace nhttp {

namespace {

    const char *authenticate_hdrs[] = {
        "WWW-Authenticate: ApiKey realm=\"Printer API\"\r\n",
        nullptr,
    };

    const constexpr StatusText texts[] = {
        { Status::Unknown, "???" },
        { Status::Ok, "OK" },
        { Status::NoContent, "No Content" },
        { Status::BadRequest, "Bad Request" },
        { Status::Unauthorized, "Unauthorized", authenticate_hdrs },
        { Status::NotFound, "Not Found" },
        { Status::MethodNotAllowed, "Method Not Allowed" },
        { Status::Conflict, "Conflict" },
        { Status::LengthRequired, "Length Required" },
        { Status::UnsupportedMediaType, "Unsupported Media Type" },
        { Status::IMATeaPot, "I'm a Teapot Printer" },
        { Status::UriTooLong, "URI Too Long" },
        { Status::UnprocessableEntity, "Unprocessable Entity" },
        { Status::RequestHeaderFieldsTooLarge, "Request Header Fields Too Large" },
        { Status::InternalServerError, "Infernal Server Error" },
        { Status::NotImplemented, "Not Implemented" },
        { Status::ServiceTemporarilyUnavailable, "Service Temporarily Unavailable" },
        { Status::InsufficientStorage, "Insufficient Storage" },
    };

    constexpr const size_t content_buffer_len = 128;

    const char *content_type_string(ContentType content_type) {
        switch (content_type) {
        case ContentType::TextPlain:
            return "text/plain";
        case ContentType::TextHtml:
            return "text/html; charset=utf-8";
        case ContentType::TextCss:
            return "text/css";
        case ContentType::ImageIco:
            return "image/vnd.microsoft.icon";
        case ContentType::ImagePng:
            return "image/png";
        case ContentType::ImageSvg:
            return "image/xml+svg";
        case ContentType::ApplicationJavascript:
            return "application/javascript";
        case ContentType::ApplicationJson:
            return "application/json";
        case ContentType::ApplicationOctetStream:
            return "application/octet-stream";
        default:
            assert(0);
            return "application/octet-stream";
        }
    }
}

const StatusText &StatusText::find(Status status) {
    for (const StatusText *t = texts; t < (texts + sizeof(texts) / sizeof(*texts)); t++) {
        if (t->status == status) {
            return *t;
        }
    }

    return *texts;
}

size_t write_headers(uint8_t *buffer, size_t buffer_len, Status status, ContentType content_type, ConnectionHandling handling, std::optional<uint64_t> content_length, const char **extra_hdrs) {
    // Always leave space for the body-newline separator
    assert(buffer_len > 2);
    buffer_len -= 2;

    char *buf = reinterpret_cast<char *>(buffer);
    const StatusText &text = StatusText::find(status);
    size_t pos = snprintf(buf, buffer_len,
        "HTTP/1.1 %u %s\r\n"
        "Content-Type: %s\r\n"
        "Connection: %s\r\n",
        static_cast<unsigned>(status),
        text.text,
        content_type_string(content_type),
        handling == ConnectionHandling::Close ? "close" : "keep-alive");
    // snprintf likes to return how much it _would_ print were there enough space.
    pos = std::min(buffer_len, pos);
    if (content_length.has_value() && pos < buffer_len) {
        pos += snprintf(buf + pos, buffer_len - pos, "Content-Length: %" PRIu64 "\r\n", *content_length);
        pos = std::min(buffer_len, pos);
    }
    if (handling == ConnectionHandling::ChunkedKeep) {
        pos += snprintf(buf + pos, buffer_len - pos, "Transfer-Encoding: chunked\r\n");
        pos = std::min(buffer_len, pos);
    }
    for (; extra_hdrs && *extra_hdrs; extra_hdrs++) {
        size_t copy = std::min(buffer_len - pos, strlen(*extra_hdrs));
        memcpy(buf + pos, *extra_hdrs, copy);
        pos += copy;
    }

    memcpy(buf + pos, "\r\n", 2);
    pos += 2;
    return pos;
}

ContentType guess_content_by_ext(const char *filename) {
    const char *last_dot = rindex(filename, '.');
    if (!last_dot) {
        return ContentType::ApplicationOctetStream;
    }

    const char *ext = last_dot + 1;

    if (strcasecmp(ext, "html") == 0 || strcasecmp(ext, "htm") == 0) {
        return ContentType::TextHtml;
    } else if (strcasecmp(ext, "css") == 0) {
        return ContentType::TextCss;
    } else if (strcasecmp(ext, "js") == 0) {
        return ContentType::ApplicationJavascript;
    } else if (strcasecmp(ext, "ico") == 0) {
        return ContentType::ImageIco;
    } else if (strcasecmp(ext, "png") == 0) {
        return ContentType::ImagePng;
    } else if (strcasecmp(ext, "svg") == 0) {
        return ContentType::ImageSvg;
    }

    return ContentType::ApplicationOctetStream;
}

}