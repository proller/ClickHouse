#include "InterserverIOHTTPHandler.h"
#include <DB/Interpreters/InterserverIOHandler.h>
#include <DB/IO/WriteBufferFromHTTPServerResponse.h>
#include <DB/IO/CompressedWriteBuffer.h>
#include <DB/IO/ReadBufferFromIStream.h>


namespace DB
{

namespace ErrorCodes
{
    extern const int ABORTED;
    extern const int POCO_EXCEPTION;
    extern const int STD_EXCEPTION;
    extern const int UNKNOWN_EXCEPTION;
    extern const int TOO_MUCH_SIMULTANEOUS_QUERIES;
}

const int HTTP_BANDWIDTH_LIMIT_EXCEEDED = 509;

void InterserverIOHTTPHandler::processQuery(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response)
{
    HTMLForm params(request);

    LOG_TRACE(log, "Request URI: " << request.getURI());

    /// NOTE: You can do authentication here if you need to.

    String endpoint_name = params.get("endpoint");
    bool compress = params.get("compress") == "true";

    ReadBufferFromIStream body(request.stream());

    WriteBufferFromHTTPServerResponse out(response);

    auto endpoint = server.global_context->getInterserverIOHandler().getEndpoint(endpoint_name);

    if (compress)
    {
        CompressedWriteBuffer compressed_out(out);
        endpoint->processQuery(params, body, compressed_out, response);
    }
    else
    {
        endpoint->processQuery(params, body, out, response);
    }

    out.finalize();
}


void InterserverIOHTTPHandler::handleRequest(Poco::Net::HTTPServerRequest & request, Poco::Net::HTTPServerResponse & response)
{
    /// In order to work keep-alive.
    if (request.getVersion() == Poco::Net::HTTPServerRequest::HTTP_1_1)
        response.setChunkedTransferEncoding(true);

    try
    {
        processQuery(request, response);
        LOG_INFO(log, "Done processing query");
    }
    catch (Exception & e)
    {

		if (e.code() == ErrorCodes::TOO_MUCH_SIMULTANEOUS_QUERIES) {
std::cerr << "toooooo many " << getCurrentExceptionMessage(0) << "\n";
			//response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_PAYMENT_REQUIRED);
			//response.setStatus(std::to_string(HTTP_BANDWIDTH_LIMIT_EXCEEDED));
			//response.setChunkedTransferEncoding(false);
			//response.setContentLength(0);
			//response.setStatus(std::to_string(HTTP_BANDWIDTH_LIMIT_EXCEEDED));
			//response.setReason("toooooooooooooooooomanyfeeeeeeethesssssssss");
std::cerr << "sst" << response.getStatus() << "\n";
			
			//response.send() << "TOO_MUCH_SIMULTANEOUS" << std::endl;
			if (!response.sent())
				response.send();
			return;
		}


        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);

        /// Sending to remote server was cancelled due to server shutdown or drop table.
        bool is_real_error = e.code() != ErrorCodes::ABORTED;

        std::string message = getCurrentExceptionMessage(is_real_error);
        if (!response.sent())
            response.send() << message << std::endl;

        if (is_real_error)
            LOG_ERROR(log, message);
        else
            LOG_INFO(log, message);
    }
    catch (...)
    {
        response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        std::string message = getCurrentExceptionMessage(false);
        if (!response.sent())
            response.send() << message << std::endl;
        LOG_ERROR(log, message);
    }
}


}
