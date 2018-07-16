#include "resource_util.h"
#include <iomanip>
#include "resource.h"
#include "include/base/cef_logging.h"
#include "include/cef_stream.h"
#include "include/cef_parser.h"
#include "include/wrapper/cef_byte_read_handler.h"
#include "include/wrapper/cef_stream_resource_handler.h"

namespace browser {

namespace {




}  // namespace


std::string GetTimeString(const CefTime& value)
{
	if (value.GetTimeT() == 0)
		return "Unspecified";

	static const char* kMonths[] = {
		"January", "February", "March", "April", "May", "June", "July", "August",
		"September", "October", "November", "December"
	};
	std::string month;
	if (value.month >= 1 && value.month <= 12)
		month = kMonths[value.month - 1];
	else
		month = "Invalid";

	std::stringstream ss;
	ss << month << " " << value.day_of_month << ", " << value.year << " " <<
		std::setfill('0') << std::setw(2) << value.hour << ":" <<
		std::setfill('0') << std::setw(2) << value.minute << ":" <<
		std::setfill('0') << std::setw(2) << value.second;
	return ss.str();
}

std::string GetCertStatusString(cef_cert_status_t status)
{
#define FLAG(flag) if (status & flag) result += std::string(#flag) + "<br/>"
	std::string result;

	FLAG(CERT_STATUS_COMMON_NAME_INVALID);
	FLAG(CERT_STATUS_DATE_INVALID);
	FLAG(CERT_STATUS_AUTHORITY_INVALID);
	FLAG(CERT_STATUS_NO_REVOCATION_MECHANISM);
	FLAG(CERT_STATUS_UNABLE_TO_CHECK_REVOCATION);
	FLAG(CERT_STATUS_REVOKED);
	FLAG(CERT_STATUS_INVALID);
	FLAG(CERT_STATUS_WEAK_SIGNATURE_ALGORITHM);
	FLAG(CERT_STATUS_NON_UNIQUE_NAME);
	FLAG(CERT_STATUS_WEAK_KEY);
	FLAG(CERT_STATUS_PINNED_KEY_MISSING);
	FLAG(CERT_STATUS_NAME_CONSTRAINT_VIOLATION);
	FLAG(CERT_STATUS_VALIDITY_TOO_LONG);
	FLAG(CERT_STATUS_IS_EV);
	FLAG(CERT_STATUS_REV_CHECKING_ENABLED);
	FLAG(CERT_STATUS_SHA1_SIGNATURE_PRESENT);
	FLAG(CERT_STATUS_CT_COMPLIANCE_FAILED);

	if (result.empty())
		return "&nbsp;";
	return result;
}

std::string GetBinaryString(CefRefPtr<CefBinaryValue> value)
{
	if (!value.get())
		return "&nbsp;";

	// Retrieve the value.
	const size_t size = value->GetSize();
	std::string src;
	src.resize(size);
	value->GetData(const_cast<char*>(src.data()), size, 0);

	// Encode the value.
	return CefBase64Encode(src.data(), src.size());
}


std::string DumpRequestContents(CefRefPtr<CefRequest> request) {
	std::stringstream ss;

	ss << "URL: " << std::string(request->GetURL());
	ss << "\nMethod: " << std::string(request->GetMethod());

	CefRequest::HeaderMap headerMap;
	request->GetHeaderMap(headerMap);
	if (headerMap.size() > 0) {
		ss << "\nHeaders:";
		CefRequest::HeaderMap::const_iterator it = headerMap.begin();
		for (; it != headerMap.end(); ++it) {
			ss << "\n\t" << std::string((*it).first) << ": " <<
				std::string((*it).second);
		}
	}

	CefRefPtr<CefPostData> postData = request->GetPostData();
	if (postData.get()) {
		CefPostData::ElementVector elements;
		postData->GetElements(elements);
		if (elements.size() > 0) {
			ss << "\nPost Data:";
			CefRefPtr<CefPostDataElement> element;
			CefPostData::ElementVector::const_iterator it = elements.begin();
			for (; it != elements.end(); ++it) {
				element = (*it);
				if (element->GetType() == PDE_TYPE_BYTES) {
					// the element is composed of bytes
					ss << "\n\tBytes: ";
					if (element->GetBytesCount() == 0) {
						ss << "(empty)";
					}
					else {
						// retrieve the data.
						size_t size = element->GetBytesCount();
						char* bytes = new char[size];
						element->GetBytes(size, bytes);
						ss << std::string(bytes, size);
						delete[] bytes;
					}
				}
				else if (element->GetType() == PDE_TYPE_FILE) {
					ss << "\n\tFile: " << std::string(element->GetFile());
				}
			}
		}
	}

	return ss.str();
}

bool LoadBinaryResource(int binaryId, DWORD &dwSize, LPBYTE &pBytes) {
	HINSTANCE hInst = GetModuleHandle(NULL);
	HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(binaryId),
		MAKEINTRESOURCE(256));
	if (hRes) {
		HGLOBAL hGlob = LoadResource(hInst, hRes);
		if (hGlob) {
			dwSize = SizeofResource(hInst, hRes);
			pBytes = (LPBYTE)LockResource(hGlob);
			if (dwSize > 0 && pBytes)
				return true;
		}
	}

	return false;
}

int GetResourceId(const char* resource_name) {
	// Map of resource labels to BINARY id values.
	static struct _resource_map {
		char* name;
		int id;
	} resource_map[] = {
		{ "logo.png", 1001 },
		//{"pdf.html", IDS_PDF_HTML},
		//{"pdf.pdf", IDS_PDF_PDF},
	};

	for (int i = 0; i < sizeof(resource_map) / sizeof(_resource_map); ++i) {
		if (!strcmp(resource_map[i].name, resource_name))
			return resource_map[i].id;
	}

	return 0;
}

// Provider of binary resources.
class BinaryResourceProvider : public CefResourceManager::Provider {
public:
	explicit BinaryResourceProvider(const std::string& url_path)
		: url_path_(url_path) {
		DCHECK(!url_path.empty());
	}

	bool OnRequest(scoped_refptr<CefResourceManager::Request> request) OVERRIDE{
		CEF_REQUIRE_IO_THREAD();

		const std::string& url = request->url();
		if (url.find(url_path_) != 0L) {
			// Not handled by this provider.
			return false;
		}

		CefRefPtr<CefResourceHandler> handler;

		const std::string& relative_path = url.substr(url_path_.length());
		if (!relative_path.empty()) {
			CefRefPtr<CefStreamReader> stream =
				GetBinaryResourceReader(relative_path.data());
			if (stream.get()) {
				handler = new CefStreamResourceHandler(
					request->mime_type_resolver().Run(url),
					stream);
			}
		}

		request->Continue(handler);
		return true;
	}

private:
	std::string url_path_;

	DISALLOW_COPY_AND_ASSIGN(BinaryResourceProvider);
};

bool LoadBinaryResource(const char* resource_name, std::string& resource_data) {
  int resource_id = GetResourceId(resource_name);
  if (resource_id == 0)
    return false;

  DWORD dwSize;
  LPBYTE pBytes;

  if (LoadBinaryResource(resource_id, dwSize, pBytes)) {
    resource_data = std::string(reinterpret_cast<char*>(pBytes), dwSize);
    return true;
  }

  NOTREACHED();  // The resource should be found.
  return false;
}

CefRefPtr<CefStreamReader> GetBinaryResourceReader(const char* resource_name) {
  int resource_id = GetResourceId(resource_name);
  if (resource_id == 0)
    return NULL;

  DWORD dwSize;
  LPBYTE pBytes;

  if (LoadBinaryResource(resource_id, dwSize, pBytes)) {
    return CefStreamReader::CreateForHandler(
        new CefByteReadHandler(pBytes, dwSize, NULL));
  }

  NOTREACHED();  // The resource should be found.
  return NULL;
}

CefResourceManager::Provider* CreateBinaryResourceProvider(
    const std::string& url_path) {
  return new BinaryResourceProvider(url_path);
}

}  // namespace client
