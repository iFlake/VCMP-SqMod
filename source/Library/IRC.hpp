#ifndef _LIBRARY_IRC_HPP_
#define _LIBRARY_IRC_HPP_

// ------------------------------------------------------------------------------------------------
#include "Base/Shared.hpp"
#include "Library/Time.hpp"
#include "Library/Numeric.hpp"

// ------------------------------------------------------------------------------------------------
#include <libircclient.h>
#include <libirc_rfcnumeric.h>

// ------------------------------------------------------------------------------------------------
#include <vector>

// ------------------------------------------------------------------------------------------------
namespace SqMod {
namespace IRC {

/* ------------------------------------------------------------------------------------------------
 * Manages a single connection to an IRC network.
*/
class Session
{
public:

    // --------------------------------------------------------------------------------------------
    typedef std::vector< Session* > Sessions; /* The type of container to store sessions. */

    /* --------------------------------------------------------------------------------------------
     * Process all sessions.
    */
    static void Process();

    /* --------------------------------------------------------------------------------------------
      * Terminate all sessions and release their resources.
     */
    static void Terminate();

protected:

    /* --------------------------------------------------------------------------------------------
     * Structure that tells the IRC library what functions to call on which events.
    */
    static irc_callbacks_t  s_Callbacks;

    /* --------------------------------------------------------------------------------------------
     * Whether the callbacks structure was initialized or not.
    */
    static bool             s_Initialized;

    /* --------------------------------------------------------------------------------------------
     * The primary session if only a single session was created.
    */
    static Session*         s_Session;

    /* --------------------------------------------------------------------------------------------
     * All the sessions if more than a session was created.
    */
    static Sessions         s_Sessions;

    /* --------------------------------------------------------------------------------------------
     * Retrieve a pre-configured callbacks structure to be assigned to a session.
    */
    static irc_callbacks_t * GetCallbacks();

protected:

    /* --------------------------------------------------------------------------------------------
     * Pool for events on the managed session.
    */
    void Update();

    /* --------------------------------------------------------------------------------------------
     * Release session resources.
    */
    void Release();

    /* --------------------------------------------------------------------------------------------
     *
    */
    bool Validate() const
    {
        if (m_Session)
            return true;
        SqThrow("Invalid IRC session (%s)", m_Tag.c_str());
        return false;
    }

    /* --------------------------------------------------------------------------------------------
     *
    */
    bool Connected() const
    {
        return (m_Session && irc_is_connected(m_Session));
    }

    /* --------------------------------------------------------------------------------------------
     *
    */
    void Destroy();

    /* --------------------------------------------------------------------------------------------
     *
    */
    bool ConnectedThrow() const
    {
        if (!m_Session)
            SqThrow("Invalid IRC session (%s)", m_Tag.c_str());
        else if (!irc_is_connected(m_Session))
            SqThrow("Session is not connected (%s)", m_Tag.c_str());
        else
            return true;
        return false;
    }

    /* --------------------------------------------------------------------------------------------
     *
    */
    bool NotConnected() const
    {
        if (!m_Session || !irc_is_connected(m_Session) || !m_Reconnect)
            return true;
        SqThrow("Already connected or trying connect to IRC server (%s)", m_Tag.c_str());
        return !m_Session;
    }

    /* --------------------------------------------------------------------------------------------
     *
    */
    static bool ValidateEventSession(Session * ptr)
    {
        if (ptr)
            return true;
        LogErr("Cannot forward IRC event without a session container");
        return false;
    }

private:

    /* --------------------------------------------------------------------------------------------
     * Copy constructor. (disabled)
    */
    Session(const Session &);

    /* --------------------------------------------------------------------------------------------
     * Copy assignment operator. (disabled)
    */
    Session & operator = (const Session &);

private:

    // --------------------------------------------------------------------------------------------
    irc_session_t*  m_Session; /* The managed IRC session structure. */

    // --------------------------------------------------------------------------------------------
    String          m_Server; /* Server address. */
    String          m_Passwd; /* Account password. */
    String          m_Nick; /* Nickname. */
    String          m_User; /* User name. */
    String          m_Name; /* Real name. */

    // --------------------------------------------------------------------------------------------
    Int32           m_Port; /* Server port. */

    // --------------------------------------------------------------------------------------------
    Int32           m_LastCode; /* Last error code that could not be returned directly. */
    Uint32          m_PoolTime; /* How much time to wait when pooling for session events. */
    Uint32          m_Tries; /* How many times to retry connection. */
    Uint32          m_Wait; /* How many milliseconds to wait between each try. */
    Uint32          m_LeftTries; /* How many tries are left. */
    Int64           m_NextTry; /* When should the session attempt to connect again. */

    // --------------------------------------------------------------------------------------------
    Int64           m_SessionTime; /* The time when the session was created. */

    // --------------------------------------------------------------------------------------------
    bool            m_Reconnect; /* Whether the session should try to reconnect. */

    // --------------------------------------------------------------------------------------------
    bool            m_IPv6; /* Whether the session was connected to an ipv6 address. */

    /* --------------------------------------------------------------------------------------------
     * Script callbacks.
    */
    Function        m_OnConnect;
    Function        m_OnNick;
    Function        m_OnQuit;
    Function        m_OnJoin;
    Function        m_OnPart;
    Function        m_OnMode;
    Function        m_OnUmode;
    Function        m_OnTopic;
    Function        m_OnKick;
    Function        m_OnChannel;
    Function        m_OnPrivMsg;
    Function        m_OnNotice;
    Function        m_OnChannelNotice;
    Function        m_OnInvite;
    Function        m_OnCtcpReq;
    Function        m_OnCtcpRep;
    Function        m_OnCtcpAction;
    Function        m_OnUnknown;
    Function        m_OnNumeric;
    Function        m_OnDccChatReq;
    Function        m_OnDccSendReq;

    // --------------------------------------------------------------------------------------------
    String          m_Tag; /* User tag. */
    Object          m_Data; /* User data. */

public:

    /* --------------------------------------------------------------------------------------------
     * Default constructor.
    */
    Session();

    /* --------------------------------------------------------------------------------------------
     * Destructor.
    */
    ~Session();

    /* --------------------------------------------------------------------------------------------
     * Used by the script engine to compare two instances of this type.
    */
    Int32 Cmp(const Session & o) const
    {
        if (m_Session == o.m_Session)
            return 0;
        else if (m_Session > o.m_Session)
            return 1;
        else
            return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * Used by the script engine to convert an instance of this type to a string.
    */
    CSStr ToString() const
    {
        return m_Server.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * See whether this session is valid.
    */
    bool IsValid() const
    {
        return m_Session;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the user tag.
    */
    CSStr GetTag() const
    {
        return m_Tag.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * Change the user tag.
    */
    void SetTag(CSStr tag)
    {
        m_Tag.assign(tag);
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the user data.
    */
    Object & GetData()
    {
        return m_Data;
    }

    /* --------------------------------------------------------------------------------------------
     * Change the user data.
    */
    void SetData(Object & data)
    {
        if (Validate())
            m_Data = data;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the server address.
    */
    CSStr GetServer() const
    {
        return m_Server.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the server address.
    */
    void SetServer(CSStr server)
    {
        if (Validate() && NotConnected())
            m_Server.assign(server);
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the account password.
    */
    CSStr GetPassword() const
    {
        return m_Passwd.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the account password.
    */
    void SetPassword(CSStr passwd)
    {
        if (Validate() && NotConnected())
            m_Passwd.assign(passwd);
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the nickname.
    */
    CSStr GetNick() const
    {
        return m_Nick.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the nickname.
    */
    void SetNick(CSStr nick)
    {
        if (!nick || strlen(nick) <= 0)
            SqThrow("Invalid IRC nickname");
        else if (Connected())
            irc_cmd_nick(m_Session, nick);
        else if (Validate())
            m_Nick.assign(nick);
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the user name.
    */
    CSStr GetUser() const
    {
        return m_User.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the user name.
    */
    void SetUser(CSStr user)
    {
        if (Validate() && NotConnected())
            m_User.assign(user);
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the real name.
    */
    CSStr GetName() const
    {
        return m_Name.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the real name.
    */
    void SetName(CSStr name)
    {
        if (Validate() && NotConnected())
            m_Name.c_str();
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the server port number.
    */
    Uint32 GetPort() const
    {
        return m_Port;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the server port number.
    */
    void SetPort(Uint32 num)
    {
        if (num > NumLimit< Uint16 >::Max)
            SqThrow("Port number is out of range: %u > %u", num, NumLimit< Uint16 >::Max);
        else if (Validate() && NotConnected())
            m_Port = num;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the amount of time to pool for session events.
    */
    Uint32 GetPoolTime() const
    {
        return m_PoolTime;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the amount of time to pool for session events.
    */
    void SetPoolTime(Uint32 num)
    {
        m_PoolTime = num;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the last error code that could not be returned directly.
    */
    Int32 GetLastCode() const
    {
        return m_LastCode;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the amount of connection attempts.
    */
    Uint32 GetTries() const
    {
        return m_Tries;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the amount of connection attempts.
    */
    void SetTries(Uint32 num)
    {
        m_Tries = num;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the amount of time in milliseconds to wait between connection attempts.
    */
    Uint32 GetWait() const
    {
        return m_Wait;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify the amount of time in milliseconds to wait between connection attempts.
    */
    void SetWait(Uint32 ms)
    {
        m_Wait = ms;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve how many times are left to try to connect.
    */
    Uint32 GetLeftTries() const
    {
        return m_LeftTries;
    }

    /* --------------------------------------------------------------------------------------------
     * Modify how many times are left to try to connect.
    */
    void SetLeftTries(Uint32 num)
    {
        m_LeftTries = num;
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve when the next connection retry should be performed.
    */
    Timestamp GetNextTry() const
    {
        return Timestamp(m_NextTry);
    }

    /* --------------------------------------------------------------------------------------------
     * Modify when the next connection retry should be performed.
    */
    void SetNextTry(const Timestamp & tm)
    {
        m_NextTry = tm.GetMicroseconds().GetNum();
    }

    /* --------------------------------------------------------------------------------------------
     * Retrieve the session uptime.
    */
    Timestamp GetSessionTime() const
    {
        if (m_SessionTime)
            return Timestamp(GetEpochTimeMicro() - m_SessionTime);
        return Timestamp();
    }

    /* --------------------------------------------------------------------------------------------
     * See whether the session is currently trying to reconnect.
    */
    bool GetReconnect() const
    {
        return m_Reconnect;
    }

    /* --------------------------------------------------------------------------------------------
     * See whether the session is connected to an ipv6 address.
    */
    bool GetIPv6() const
    {
        return m_IPv6;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect();

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect(CSStr server, Uint32 port, CSStr nick)
    {
        return Connect(server, port, nick, NULL, NULL, NULL);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect(CSStr server, Uint32 port, CSStr nick, CSStr passwd)
    {
        return Connect(server, port, nick, passwd, NULL, NULL);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect(CSStr server, Uint32 port, CSStr nick, CSStr passwd, CSStr user)
    {
        return Connect(server, port, nick, passwd, user, NULL);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect(CSStr server, Uint32 port, CSStr nick, CSStr passwd, CSStr user, CSStr name);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect6();

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect6(CSStr server, Uint32 port, CSStr nick)
    {
        return Connect(server, port, nick, NULL, NULL, NULL);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect6(CSStr server, Uint32 port, CSStr nick, CSStr passwd)
    {
        return Connect(server, port, nick, passwd, NULL, NULL);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect6(CSStr server, Uint32 port, CSStr nick, CSStr passwd, CSStr user)
    {
        return Connect(server, port, nick, passwd, user, NULL);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 Connect6(CSStr server, Uint32 port, CSStr nick, CSStr passwd, CSStr user, CSStr name);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void Disconnect();

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    bool IsConnected()
    {
        return Connected();
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdJoin(CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_join(m_Session, channel, NULL);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdJoin(CSStr channel, CSStr key)
    {
        if (ConnectedThrow())
            return irc_cmd_join(m_Session, channel, key);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdPart(CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_part(m_Session, channel);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdInvite(CSStr nick, CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_invite(m_Session, nick, channel);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdNames(CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_names(m_Session, channel);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdList()
    {
        if (ConnectedThrow())
            return irc_cmd_list(m_Session, NULL);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdList(CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_list(m_Session, channel);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdTopic(CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_topic(m_Session, channel, NULL);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdTopic(CSStr channel, CSStr topic)
    {
        if (ConnectedThrow())
            return irc_cmd_topic(m_Session, channel, topic);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdChannelMode(CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_channel_mode(m_Session, channel, NULL);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdChannelMode(CSStr channel, CSStr mode)
    {
        if (ConnectedThrow())
            return irc_cmd_channel_mode(m_Session, channel, mode);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdUserMode()
    {
        if (ConnectedThrow())
            return irc_cmd_user_mode(m_Session, NULL);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdUserMode(CSStr mode)
    {
        if (ConnectedThrow())
            return irc_cmd_user_mode(m_Session, mode);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdKick(CSStr nick, CSStr channel)
    {
        if (ConnectedThrow())
            return irc_cmd_kick(m_Session, nick, channel, NULL);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdKick(CSStr nick, CSStr channel, CSStr reason)
    {
        if (ConnectedThrow())
            return irc_cmd_kick(m_Session, nick, channel, reason);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdMsg(CSStr nch, CSStr text)
    {
        if (ConnectedThrow())
            return irc_cmd_msg(m_Session, nch, text);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdMe(CSStr nch, CSStr text)
    {
        if (ConnectedThrow())
            return irc_cmd_me(m_Session, nch, text);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdNotice(CSStr nch, CSStr text)
    {
        if (ConnectedThrow())
            return irc_cmd_notice(m_Session, nch, text);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdCtcpRequest(CSStr nick, CSStr request)
    {
        if (ConnectedThrow())
            return irc_cmd_notice(m_Session, nick, request);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdCtcpReply(CSStr nick, CSStr reply)
    {
        if (ConnectedThrow())
            return irc_cmd_ctcp_reply(m_Session, nick, reply);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdNick(CSStr nick)
    {
        if (!nick || strlen(nick) <= 0)
            SqThrow("Invalid IRC nickname");
        else if (ConnectedThrow())
            return irc_cmd_nick(m_Session, nick);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdWhois(CSStr nick)
    {
        if (ConnectedThrow())
            return irc_cmd_whois(m_Session, nick);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdQuit()
    {
        if (ConnectedThrow())
            return irc_cmd_quit(m_Session, NULL);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 CmdQuit(CSStr reason)
    {
        if (ConnectedThrow())
            return irc_cmd_quit(m_Session, reason);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 SendRaw(CSStr str)
    {
        if (ConnectedThrow())
            return irc_send_raw(m_Session, str);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 DestroyDcc(Uint32 dccid)
    {
        if (ConnectedThrow())
            return irc_dcc_destroy(m_Session, dccid);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetCtcpVersion(CSStr version)
    {
        if (ConnectedThrow())
            irc_set_ctcp_version(m_Session, version);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Int32 GetErrNo()
    {
        if (Validate())
            return irc_errno(m_Session);
        return -1;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    CSStr GetErrStr()
    {
        if (Validate())
            return irc_strerror(irc_errno(m_Session));
        return _SC("");
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void SetOption(Uint32 option)
    {
        if (Validate())
            return irc_option_set(m_Session, option);
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void ResetOption(Uint32 option)
    {
        if (Validate())
            return irc_option_set(m_Session, option);
    }

protected:

    /* --------------------------------------------------------------------------------------------
     * Forward session events to a script callback.
    */
    static void ForwardEvent(Session * session, Function & listener, CCStr event,
                                CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * Forward session events to a script callback.
    */
    static void ForwardEvent(Session * session, Function & listener, Uint32 event,
                                CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * Forward session events to a script callback.
    */
    static void ForwardEvent(Session * session, Function & listener, CCStr nick,
                                CCStr addr, irc_dcc_t dccid);

    /* --------------------------------------------------------------------------------------------
     * Forward session events to a script callback.
    */
    static void ForwardEvent(Session * session, Function & listener, CCStr nick,
                                CCStr addr, CCStr filename, Ulong size, irc_dcc_t dccid);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnConnect(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnNick(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnQuit(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnJoin(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnPart(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnMode(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnUmode(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnTopic(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnKick(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnChannel(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnPrivMsg(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnNotice(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnChannelNotice(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnInvite(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnCtcpReq(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnCtcpRep(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnCtcpAction(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnUnknown(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnNumeric(irc_session_t * session, Uint32 event, CCStr origin, CCStr * params, Uint32 count);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnDccChatReq(irc_session_t * session, CCStr nick, CCStr addr, irc_dcc_t dccid);

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    static void OnDccSendReq(irc_session_t * session, CCStr nick, CCStr addr, CCStr filename, Ulong size, irc_dcc_t dccid);

public:

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnConnect()
    {
        return m_OnConnect;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnConnect(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnConnect.ReleaseGently();
        else
            m_OnConnect = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnNick()
    {
        return m_OnNick;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnNick(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnNick.ReleaseGently();
        else
            m_OnNick = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnQuit()
    {
        return m_OnQuit;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnQuit(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnQuit.ReleaseGently();
        else
            m_OnQuit = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnJoin()
    {
        return m_OnJoin;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnJoin(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnJoin.ReleaseGently();
        else
            m_OnJoin = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnPart()
    {
        return m_OnPart;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnPart(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnPart.ReleaseGently();
        else
            m_OnPart = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnMode()
    {
        return m_OnMode;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnMode(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnMode.ReleaseGently();
        else
            m_OnMode = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnUmode()
    {
        return m_OnUmode;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnUmode(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnUmode.ReleaseGently();
        else
            m_OnUmode = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnTopic()
    {
        return m_OnTopic;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnTopic(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnTopic.ReleaseGently();
        else
            m_OnTopic = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnKick()
    {
        return m_OnKick;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnKick(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnKick.ReleaseGently();
        else
            m_OnKick = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnChannel()
    {
        return m_OnChannel;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnChannel(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnChannel.ReleaseGently();
        else
            m_OnChannel = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnPrivMsg()
    {
        return m_OnPrivMsg;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnPrivMsg(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnPrivMsg.ReleaseGently();
        else
            m_OnPrivMsg = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnNotice()
    {
        return m_OnNotice;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnNotice(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnNotice.ReleaseGently();
        else
            m_OnNotice = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnChannelNotice()
    {
        return m_OnChannelNotice;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnChannelNotice(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnChannelNotice.ReleaseGently();
        else
            m_OnChannelNotice = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnInvite()
    {
        return m_OnInvite;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnInvite(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnInvite.ReleaseGently();
        else
            m_OnInvite = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnCtcpReq()
    {
        return m_OnCtcpReq;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnCtcpReq(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnCtcpReq.ReleaseGently();
        else
            m_OnCtcpReq = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnCtcpRep()
    {
        return m_OnCtcpRep;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnCtcpRep(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnCtcpRep.ReleaseGently();
        else
            m_OnCtcpRep = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnCtcpAction()
    {
        return m_OnCtcpAction;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnCtcpAction(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnCtcpAction.ReleaseGently();
        else
            m_OnCtcpAction = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnUnknown()
    {
        return m_OnUnknown;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnUnknown(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnUnknown.ReleaseGently();
        else
            m_OnUnknown = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnNumeric()
    {
        return m_OnNumeric;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnNumeric(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnNumeric.ReleaseGently();
        else
            m_OnNumeric = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnDccChatReq()
    {
        return m_OnDccChatReq;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnDccChatReq(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnDccChatReq.ReleaseGently();
        else
            m_OnDccChatReq = Function(env.GetVM(), env, func.GetFunc());
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    Function & GetOnDccSendReq()
    {
        return m_OnDccSendReq;
    }

    /* --------------------------------------------------------------------------------------------
     * ...
    */
    void BindOnDccSendReq(Object & env, Function & func)
    {
        if (func.IsNull())
            m_OnDccSendReq.ReleaseGently();
        else
            m_OnDccSendReq = Function(env.GetVM(), env, func.GetFunc());
    }
};

} // Namespace:: IRC
} // Namespace:: SqMod

#endif // _LIBRARY_IRC_HPP_