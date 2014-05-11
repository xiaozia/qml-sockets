
#ifndef QML_SOCKETS_TCP
#define QML_SOCKETS_TCP

#include <QtNetwork>


class TCPSocket : public QObject
{
  Q_OBJECT
  Q_ENUMS(QAbstractSocket::SocketState)
  
  Q_PROPERTY(QString host    MEMBER m_host    NOTIFY hostChanged)
  Q_PROPERTY(uint    port    MEMBER m_port    NOTIFY portChanged)
  Q_PROPERTY(QAbstractSocket::SocketState state
                 MEMBER m_state       NOTIFY stateChanged)
  
  Q_PROPERTY(QRegExp expression
                 MEMBER m_expression  NOTIFY expressionChanged)
  Q_PROPERTY(QString matchBuffer
                 MEMBER m_matchBuffer NOTIFY matchBufferChanged)
  
signals:
  void hostChanged();
  void portChanged();
  void stateChanged();
  void expressionChanged();
  void matchBufferChanged();
  
  void read(const QString& message);
  void connected();
  void disconnected();
  
  // When the expression is matched in the matchBuffer,
  // match will be signalled with the match data
  void match(
    const QString&     matchString,
    const QStringList& matchCaptures,
    const QString&     preMatchString);
  
public:
  TCPSocket(QObject* parent = 0)
  { (void)parent; assignSocket(); };
  
  ~TCPSocket()
  { delete m_socket; m_socket = NULL; }
  
  void assignSocket(QTcpSocket *socket = NULL)
  {
    // Delete old socket if existent
    if(m_socket!=NULL)
      delete m_socket;
    
    // Create new socket or assign passed socket
    if(socket!=NULL)
      m_socket = socket;
    else
      m_socket = new QTcpSocket(this);
    
    // Register event handlers
    QObject::connect(m_socket, &QAbstractSocket::stateChanged,
      [=](QAbstractSocket::SocketState state)
      { setProperty("state", state); });
    
    QObject::connect(m_socket, &QAbstractSocket::readyRead,
      [=]() { emit read(m_socket->readAll()); });
    
    QObject::connect(m_socket, &QAbstractSocket::connected,
      [=]() { m_matchBuffer.clear(); emit connected(); });
    
    QObject::connect(m_socket, &QAbstractSocket::disconnected,
      [=]() { emit disconnected(); });
    
    QObject::connect(this, &TCPSocket::read,
      [=](const QString& buffer) {
      
      // Expression matching is disabled by default to save memory (matchBuffer)
      if(!m_expression.isEmpty()) {
        // Append the new message to the buffer
        m_matchBuffer.append(buffer);
        
        // Pull out each match and pre-match and trigger the match signal
        QString str, pre;
        int idx;
        while(-1 != (idx = m_expression.indexIn(m_matchBuffer))) {
          str = m_expression.capturedTexts()[0];
          pre = m_matchBuffer.left(idx);
          m_matchBuffer = m_matchBuffer.remove(0, str.length()+idx);
          emit match(str, m_expression.capturedTexts().mid(1), pre);
        }
      }
    });
    
  }
  
public slots:
  void connect()
  { m_socket->connectToHost(m_host, m_port); }
  
  void disconnect()
  { m_socket->disconnectFromHost(); }
  
  void write(QString message)
  { m_socket->write(message.toLocal8Bit()); }
  
public:
  QString m_host;
  uint    m_port;
  QAbstractSocket::SocketState m_state;
  QRegExp m_expression;
  QString m_matchBuffer;
  
  QTcpSocket *m_socket = NULL;
};

#endif
