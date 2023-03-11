#include "authentication.h"

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QtNetworkAuth>
#include <QMessageBox>

const QString Authentication::GOOGLE_API = "https://www.googleapis.com";

Authentication::Authentication(QObject *parent)
	: QObject(parent)
	,m_authenticated(false)
{
	google = new QOAuth2AuthorizationCodeFlow(this);
	connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
}

bool Authentication::checkAuth()
{
	if (!m_authenticated) {
		QTimer waitTimer;
		QEventLoop waitAuthLoop;
		connect(google, &QOAuth2AuthorizationCodeFlow::granted, &waitTimer, &QTimer::stop);
		connect(google, &QOAuth2AuthorizationCodeFlow::granted, &waitAuthLoop, &QEventLoop::quit);
		connect(&waitTimer, &QTimer::timeout, &waitAuthLoop, &QEventLoop::quit);
		connect(this, &Authentication::cancel, &waitAuthLoop, &QEventLoop::quit);
		if (!authorize()) {
			emit alert(tr("Check client_id.json"));
			return false;
		}
		waitTimer.start(AUTH_TIMEOUT);
		waitAuthLoop.exec();
		if (waitTimer.isActive()) {
			emit alert(tr("Authentication timeout..."));
			return false;
		}
		emit authorized();
		m_authenticated = true;
	}
	return true;
}

bool Authentication::isAuthenticated() const
{
	return m_authenticated;
}

void Authentication::setCredentials(const QString &path)
{
	m_credentials = path;
}

bool Authentication::authorize()
{
	if (m_credentials.isEmpty())
		return false;
	QFile file(m_credentials);
	if (!file.open(QIODevice::ReadOnly))
		return false;
	const QByteArray bytes = file.readAll();
	QJsonDocument document = QJsonDocument::fromJson(bytes);

	const auto object = document.object();
	const auto settingsObject = object["installed"].toObject();
	const QUrl authUri(settingsObject["auth_uri"].toString());
	const auto clientId = settingsObject["client_id"].toString();
	const QUrl tokenUri(settingsObject["token_uri"].toString());
	const auto clientSecret(settingsObject["client_secret"].toString());
	const auto redirectUris = settingsObject["redirect_uris"].toArray();
	const QUrl redirectUri(redirectUris[0].toString()); // Get the first URI
	const auto port = static_cast<quint16>(redirectUri.port()); // Get the port

	google->setScope(QString("%1/auth/spreadsheets.readonly").arg(GOOGLE_API));
	google->setAuthorizationUrl(authUri);
	google->setClientIdentifier(clientId);
	google->setAccessTokenUrl(tokenUri);
	google->setClientIdentifierSharedKey(clientSecret);

	auto replyHandler = new QOAuthHttpServerReplyHandler(port, this);
	google->setReplyHandler(replyHandler);

	google->grant();
	return true;
}

QByteArray Authentication::sendRequest(const QUrl &url)
{
	if (!checkAuth())
		return QByteArray();

	QNetworkRequest request(url);
	QString headerData = "Bearer " + google->token();
	request.setRawHeader("Authorization", headerData.toLocal8Bit());
	QNetworkReply* reply = google->networkAccessManager()->get(request);

	QEventLoop loop;
	connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
	loop.exec();

	QByteArray result;
	if (reply->error() == QNetworkReply::NoError)
		result = reply->readAll();
	else
		emit alert(reply->errorString());

	reply->deleteLater();
	return result;
}
