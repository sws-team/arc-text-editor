#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include <QObject>
#include <QJsonObject>
#include <QUrlQuery>

class Authentication : public QObject
{
	Q_OBJECT
public:
	Authentication(QObject* parent = nullptr);

	bool isAuthenticated() const;
	void setCredentials(const QString& path);

	static const QString GOOGLE_API;
	Q_DECL_CONSTEXPR static int AUTH_TIMEOUT = 1 * 60 * 1000;//1 min

signals:
	void alert(const QString&);
	void cancel();

public slots:
	QByteArray sendRequest(const QUrl &url);
signals:
	void authorized();
private:
	bool authorize();
	class QOAuth2AuthorizationCodeFlow* google;
	QString m_credentials;
	bool m_authenticated;
	bool checkAuth();
};

#endif // AUTHENTICATION_H
