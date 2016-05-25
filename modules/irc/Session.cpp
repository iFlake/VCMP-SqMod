// ------------------------------------------------------------------------------------------------
#include "Session.hpp"
#include "Module.hpp"

// ------------------------------------------------------------------------------------------------
#include <cstring>

// ------------------------------------------------------------------------------------------------
#include <algorithm>

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
irc_callbacks_t     Session::s_Callbacks;

// ------------------------------------------------------------------------------------------------
bool                Session::s_Initialized = false;

// ------------------------------------------------------------------------------------------------
Session*            Session::s_Session = nullptr;

// ------------------------------------------------------------------------------------------------
Session::Sessions   Session::s_Sessions;

// ------------------------------------------------------------------------------------------------
SQInteger Session::Typename(HSQUIRRELVM vm)
{
    static const SQChar name[] = _SC("SqIrcSession");
    sq_pushstring(vm, name, sizeof(name));
    return 1;
}

// ------------------------------------------------------------------------------------------------
void Session::Process()
{
    // Do we only have one IRC session?
    if (s_Session)
    {
        s_Session->Update();
    }
    // Do we have multiple sessions?
    else if (!s_Sessions.empty())
    {
        for (Sessions::iterator itr = s_Sessions.begin(); itr != s_Sessions.end(); ++itr)
        {
            (*itr)->Update();
        }
    }
}

// ------------------------------------------------------------------------------------------------
void Session::Terminate()
{
    // Do we only have one IRC session?
    if (s_Session)
    {
        s_Session->Destroy(); /* This should do the job. */
    }
    // Do we have multiple sessions?
    else if (!s_Sessions.empty())
    {
        for (Sessions::iterator itr = s_Sessions.begin(); itr != s_Sessions.end(); ++itr)
        {
            (*itr)->Destroy();
        }
    }
}

// ------------------------------------------------------------------------------------------------
irc_callbacks_t * Session::GetCallbacks()
{
    // See if the callbacks structure was initialized before
    if (!s_Initialized)
    {
        // Initialize the callbacks
        s_Callbacks.event_connect           = &OnConnect;
        s_Callbacks.event_nick              = &OnNick;
        s_Callbacks.event_quit              = &OnQuit;
        s_Callbacks.event_join              = &OnJoin;
        s_Callbacks.event_part              = &OnPart;
        s_Callbacks.event_mode              = &OnMode;
        s_Callbacks.event_umode             = &OnUmode;
        s_Callbacks.event_topic             = &OnTopic;
        s_Callbacks.event_kick              = &OnKick;
        s_Callbacks.event_channel           = &OnChannel;
        s_Callbacks.event_privmsg           = &OnPrivMsg;
        s_Callbacks.event_notice            = &OnNotice;
        s_Callbacks.event_channel_notice    = &OnChannelNotice;
        s_Callbacks.event_invite            = &OnInvite;
        s_Callbacks.event_ctcp_req          = &OnCtcpReq;
        s_Callbacks.event_ctcp_rep          = &OnCtcpRep;
        s_Callbacks.event_ctcp_action       = &OnCtcpAction;
        s_Callbacks.event_unknown           = &OnUnknown;
        s_Callbacks.event_numeric           = &OnNumeric;
        s_Callbacks.event_dcc_chat_req      = &OnDccChatReq;
        s_Callbacks.event_dcc_send_req      = &OnDccSendReq;
        // Specify that the callbacks were initialized
        s_Initialized = true;
    }
    // Return the callbacks structure
    return &s_Callbacks;
}

// ------------------------------------------------------------------------------------------------
void Session::Update()
{
    // Make sure we even have a session
    if (!m_Session)
    {
        return;
    }
    // Make sure that the IRC session is connected
    else if (!irc_is_connected(m_Session))
    {
        // Do we meet the condition to attempt to reconnect?
        if (m_Reconnect && (m_LeftTries != 0) && (m_NextTry <= _SqMod->GetEpochTimeMicro()))
        {
            // Take out one try
            --m_LeftTries;
            // Update the time-point for the next try
            m_NextTry = (_SqMod->GetEpochTimeMicro() + (m_Wait * 1000LL));
            // Attempt to reconnect
            if (m_IPv6)
            {
                m_LastCode = irc_connect6(m_Session, m_Server.c_str(), m_Port,
                            m_Passwd.empty() ? nullptr : m_Passwd.c_str(),
                            m_Nick.c_str(),
                            m_User.empty() ? nullptr : m_User.c_str(),
                            m_Name.empty() ? nullptr : m_Name.c_str()
                );
            }
            else
            {
                m_LastCode = irc_connect(m_Session, m_Server.c_str(), m_Port,
                            m_Passwd.empty() ? nullptr : m_Passwd.c_str(),
                            m_Nick.c_str(),
                            m_User.empty() ? nullptr : m_User.c_str(),
                            m_Name.empty() ? nullptr : m_Name.c_str()
                );
            }
        }
        // We're done for now
        return;
    }
    // Create the structures for select()
    struct timeval tv;
    fd_set in_set, out_set;
    int maxfd = 0;
    // Pool for the specified amount of time
    tv.tv_usec = (m_PoolTime * 1000L);
    tv.tv_sec = 0;
    // Initialize the sets
    std::memset(&in_set, 0, sizeof(fd_set));
    std::memset(&out_set, 0, sizeof(fd_set));
    // Add the IRC session descriptors
    irc_add_select_descriptors(m_Session, &in_set, &out_set, &maxfd);
    // Call select()
    if (select(maxfd + 1, &in_set, &out_set, 0, &tv) < 0)
    {
        _SqMod->LogErr("Unable to select() on IRC session");
    }
    // Call irc_process_select_descriptors() for the session
    if (irc_process_select_descriptors (m_Session, &in_set, &out_set))
    {
        // @TODO: The connection failed, or the server disconnected. Handle it!
        _SqMod->LogWrn("The IRC connection failed, or the server disconnected.");
    }
}

// ------------------------------------------------------------------------------------------------
void Session::Release()
{
    // Release the reference to the specified callbacks
    m_OnConnect.ReleaseGently();
    m_OnNick.ReleaseGently();
    m_OnQuit.ReleaseGently();
    m_OnJoin.ReleaseGently();
    m_OnPart.ReleaseGently();
    m_OnMode.ReleaseGently();
    m_OnUmode.ReleaseGently();
    m_OnTopic.ReleaseGently();
    m_OnKick.ReleaseGently();
    m_OnChannel.ReleaseGently();
    m_OnPrivMsg.ReleaseGently();
    m_OnNotice.ReleaseGently();
    m_OnChannelNotice.ReleaseGently();
    m_OnInvite.ReleaseGently();
    m_OnCtcpReq.ReleaseGently();
    m_OnCtcpRep.ReleaseGently();
    m_OnCtcpAction.ReleaseGently();
    m_OnUnknown.ReleaseGently();
    m_OnNumeric.ReleaseGently();
    m_OnDccChatReq.ReleaseGently();
    m_OnDccSendReq.ReleaseGently();
    // Release the reference to the specified user data
    m_Data.Release();
}

// ------------------------------------------------------------------------------------------------
void Session::Destroy()
{
    // Make sure there's even a session to release
    if (!m_Session)
        return;
    // Disconnect the session
    Disconnect();
    // Break the association with this instance (paranoia)
    irc_set_ctx(m_Session, nullptr);
    // Destroy the IRC session structure
    irc_destroy_session(m_Session);
    // Explicitly make sure no further calls can be made to this session (again... paranoia)
    m_Session = nullptr;
    // Release resources
    Release();
}

// ------------------------------------------------------------------------------------------------
void Session::Validate() const
{
    // Do we have a valid session handle?
    if (!m_Session)
    {
        STHROWF("Invalid IRC session");
    }
}

// ------------------------------------------------------------------------------------------------
void Session::ValidateConnection() const
{
    // Do we have a valid session handle?
    if (!m_Session)
    {
        STHROWF("Invalid IRC session");
    }
    // Is the session connected?
    else if (!irc_is_connected(m_Session))
    {
        STHROWF("Session is not connected");
    }
}

// ------------------------------------------------------------------------------------------------
void Session::IsNotConnected() const
{
    // Do we have a session that is not connected or trying to connect?
    if (m_Session && (irc_is_connected(m_Session) || m_Reconnect))
    {
        STHROWF("Already connected or trying connect to IRC server");
    }
}

// ------------------------------------------------------------------------------------------------
bool Session::ValidateEventSession(Session * ptr)
{
    // Is the session instance valid?
    if (ptr)
    {
        return true;
    }
    // We can't throw an error here so we simply log it
    _SqMod->LogErr("Cannot forward IRC event without a session instance");
    // Invalid session instance
    return false;
}

// ------------------------------------------------------------------------------------------------
Session::Session()
    : m_Session(irc_create_session(GetCallbacks()))
    , m_Server(_SC(""))
    , m_Passwd(_SC(""))
    , m_Nick(_SC(""))
    , m_User(_SC(""))
    , m_Name(_SC(""))
    , m_Port(6667)
    , m_LastCode(0)
    , m_PoolTime(4)
    , m_Tries(3)
    , m_Wait(5000)
    , m_LeftTries(0)
    , m_NextTry(0)
    , m_SessionTime(0)
    , m_Reconnect(false)
    , m_IPv6(false)
{
    if (!m_Session)
    {
        // Explicitly make sure no further calls can be made to this session
        m_Session = nullptr;
        // Now it's safe to throw the error
        STHROWF("Unable to create an IRC session");
    }
    else
    {
        // Associate this wrapper instance with the session structure
        irc_set_ctx(m_Session, this);
        // Is this the only session instance?
        if (!s_Session && s_Sessions.empty())
        {
            s_Session = this;
        }
        // Is this the second session instance?
        else if (s_Sessions.empty())
        {
            s_Sessions.push_back(s_Session);
            s_Session = nullptr;
            s_Sessions.push_back(this);
        }
        // This is part of multiple session instances
        else
        {
            s_Sessions.push_back(this);
        }
    }
}

// ------------------------------------------------------------------------------------------------
Session::~Session()
{
    Destroy();
    // Attempt to find our self in the session pool
    Sessions::iterator itr = std::find(s_Sessions.begin(), s_Sessions.end(), this);
    // Are we in the pool?
    if (itr != s_Sessions.end())
    {
        s_Sessions.erase(itr); /* Remove our self from the pool. */
    }
    // Is there a single session and that's us?
    if (s_Session == this)
    {
        s_Session = nullptr;
    }
}

// ------------------------------------------------------------------------------------------------
Function & Session::GetEvent(Int32 evid)
{
    // Identify the requested event type
    switch (evid)
    {
        case SET_CONNECT:           return m_OnConnect;
        case SET_NICK:              return m_OnNick;
        case SET_QUIT:              return m_OnQuit;
        case SET_JOIN:              return m_OnJoin;
        case SET_PART:              return m_OnPart;
        case SET_MODE:              return m_OnMode;
        case SET_UMODE:             return m_OnUmode;
        case SET_TOPIC:             return m_OnTopic;
        case SET_KICK:              return m_OnKick;
        case SET_CHANNEL:           return m_OnChannel;
        case SET_PRIVMSG:           return m_OnPrivMsg;
        case SET_NOTICE:            return m_OnNotice;
        case SET_CHANNELNOTICE:     return m_OnChannelNotice;
        case SET_INVITE:            return m_OnInvite;
        case SET_CTCPREQ:           return m_OnCtcpReq;
        case SET_CTCPREP:           return m_OnCtcpRep;
        case SET_CTCPACTION:        return m_OnCtcpAction;
        case SET_UNKNOWN:           return m_OnUnknown;
        case SET_NUMERIC:           return m_OnNumeric;
        case SET_DCCCHATREQ:        return m_OnDccChatReq;
        case SET_DCCSENDREQ:        return m_OnDccSendReq;
        default: break;
    }
    // Default to a null function
    return NullFunction();
}

// ------------------------------------------------------------------------------------------------
void Session::BindEvent(Int32 evid, Object & env, Function & func)
{
    // Validate the handle
    Validate();
    // Obtain the function instance called for this event
    Function & event = GetEvent(evid);
    // Is the specified callback function null?
    if (func.IsNull())
    {
        event.ReleaseGently(); // Then release the current callback
    }
    // Does this function need a custom environment?
    else if (env.IsNull())
    {
        event = func;
    }
    // Assign the specified environment and function
    else
    {
        event = Function(env.GetVM(), env, func.GetFunc());
    }
}

// ------------------------------------------------------------------------------------------------
void Session::SetNick(CSStr nick)
{
    // Validate the handle
    Validate();
    // Validate the specified nick name
    if (!nick || *nick == '\0')
    {
        STHROWF("Invalid IRC nickname");
    }
    // Do we have to issue a nickname command?
    else if (Connected())
    {
        irc_cmd_nick(m_Session, nick);
    }
    // Simply save the specified nickname
    else
    {
        m_Nick.assign(nick);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::SetPort(Uint32 num)
{
    // The port cannot be changed once connected!
    IsNotConnected();
    // Validate the specified port number
    if (num > 0xFFFF)
    {
        STHROWF("Port number is out of range: %u > %u", num, 0xFFFF);
    }
    // Assign the specified port number
    m_Port = num;
}

// ------------------------------------------------------------------------------------------------
Int32 Session::CmdNick(CSStr nick)
{
    // Make sure the session is connected
    ValidateConnection();
    // Validate the specified nick name
    if (!nick || strlen(nick) <= 0)
    {
        STHROWF("Invalid IRC nickname");
    }
    // Issue the command and return the result
    return irc_cmd_nick(m_Session, nick);
}

// ------------------------------------------------------------------------------------------------
Object Session::GetNextTry() const
{
    // Obtain the initial stack size
    const StackGuard sg(_SqVM);
    // Attempt to push a time-stamp instance on the stack
    _SqMod->PushTimestamp(_SqVM, m_NextTry);
    // Obtain the object from the stack and return it
    return Var< Object >(_SqVM, -1).value;
}

// ------------------------------------------------------------------------------------------------
void Session::SetNextTry(Object & tm)
{
    // Obtain the initial stack size
    const StackGuard sg(_SqVM);
    // Push the specified object onto the stack
    Var< Object >::push(_SqVM, tm);
    // The resulted times-tamp value
    Int64 microseconds = 0;
    // Attempt to get the numeric value inside the specified object
    if (SQ_FAILED(_SqMod->GetTimestamp(_SqVM, -1, &microseconds)))
    {
        STHROWF("Invalid time-stamp specified");
    }
    // Assign the specified timestamp value
    m_NextTry = microseconds;
}

// ------------------------------------------------------------------------------------------------
Object Session::GetSessionTime() const
{
    // Obtain the initial stack size
    const StackGuard sg(_SqVM);
    // Attempt to push a time-stamp instance on the stack
    if (m_SessionTime)
    {
        _SqMod->PushTimestamp(_SqVM, _SqMod->GetEpochTimeMicro() - m_SessionTime);
    }
    // This session was not connected yet
    else
    {
        _SqMod->PushTimestamp(_SqVM, 0);
    }
    // Obtain the object from the stack and return it
    return Var< Object >(_SqVM, -1).value;
}

// ------------------------------------------------------------------------------------------------
Int32 Session::Connect()
{
    // Validate the handle
    Validate();
    // Make sure we are allowed to connect
    IsNotConnected();
    // Validate the specified server
    if (!m_Server.empty())
    {
        STHROWF("Attempting to connect IRC without specifying a server");
    }
    // Validate the specified nickname
    else if (!m_Nick.empty())
    {
        STHROWF("Attempting to connect IRC without specifying a nickname");
    }
    // Enable the reconnection system
    m_Reconnect = true;
    // Reset the number of tries
    m_LeftTries = m_Tries;
    // Set the time-point for the next try
    m_NextTry = (_SqMod->GetEpochTimeMicro() + (m_Wait * 1000LL));
    // This is not an IPv6 connection
    m_IPv6 = false;
    // Attempt to connect the session and return the result
    return irc_connect(m_Session, m_Server.c_str(), m_Port,
                        m_Passwd.empty() ? nullptr : m_Passwd.c_str(),
                        m_Nick.c_str(),
                        m_User.empty() ? nullptr : m_User.c_str(),
                        m_Name.empty() ? nullptr : m_Name.c_str()
    );
}

// ------------------------------------------------------------------------------------------------
Int32 Session::Connect(CSStr server, Uint32 port, CSStr nick, CSStr passwd, CSStr user, CSStr name)
{
    // Validate the handle
    Validate();
    // Make sure we are allowed to connect
    IsNotConnected();
    // Validate the specified port
    if (port > 0xFFFF)
    {
        STHROWF("Port number is out of range: %u > %u", port, 0xFFFF);
    }
    // Validate the specified server
    else if (!server || strlen(server) <= 0)
    {
        STHROWF("Attempting to connect IRC without specifying a server");
    }
    // Validate the specified nickname
    else if (!nick || strlen(nick) <= 0)
    {
        STHROWF("Attempting to connect IRC without specifying a nickname");
    }
    // Save the specified port
    m_Port = port;
    // Save the specified server
    m_Server.assign(server);
    // Save the specified nickname
    m_Nick.assign(nick);
    // Save the specified password
    m_Passwd.assign(passwd ? passwd : _SC(""));
    // Save the specified user
    m_User.assign(user ? user : _SC(""));
    // Save the specified name
    m_Name.assign(name ? name : _SC(""));
    // Enable the reconnection system
    m_Reconnect = true;
    // Reset the number of tries
    m_LeftTries = m_Tries;
    // Set the time-point for the next connection try
    m_NextTry = (_SqMod->GetEpochTimeMicro() + (m_Wait * 1000LL));
    // This is not an IPv6 connection
    m_IPv6 = false;
    // Attempt to connect the session and return the result
    return irc_connect(m_Session, m_Server.c_str(), m_Port,
                        m_Passwd.empty() ? nullptr : m_Passwd.c_str(),
                        m_Nick.c_str(),
                        m_User.empty() ? nullptr : m_User.c_str(),
                        m_Name.empty() ? nullptr : m_Name.c_str()
    );
}

// ------------------------------------------------------------------------------------------------
Int32 Session::Connect6()
{
    // Validate the handle
    Validate();
    // Make sure we are allowed to connect
    IsNotConnected();
    // Validate the specified server
    if (!m_Server.empty())
    {
        STHROWF("Attempting to connect IRC without specifying a server");
    }
    // Validate the specified nickname
    else if (!m_Nick.empty())
    {
        STHROWF("Attempting to connect IRC without specifying a nickname");
    }
    // Enable the reconnection system
    m_Reconnect = true;
    // Reset the number of tries
    m_LeftTries = m_Tries;
    // Set the time-point for the next try
    m_NextTry = (_SqMod->GetEpochTimeMicro() + (m_Wait * 1000LL));
    // This is an IPv6 connection
    m_IPv6 = true;
    // Attempt to connect the session and return the result
    return irc_connect6(m_Session, m_Server.c_str(), m_Port,
                        m_Passwd.empty() ? nullptr : m_Passwd.c_str(),
                        m_Nick.c_str(),
                        m_User.empty() ? nullptr : m_User.c_str(),
                        m_Name.empty() ? nullptr : m_Name.c_str()
    );
}

// ------------------------------------------------------------------------------------------------
Int32 Session::Connect6(CSStr server, Uint32 port, CSStr nick, CSStr passwd, CSStr user, CSStr name)
{
    // Validate the handle
    Validate();
    // Make sure we are allowed to connect
    IsNotConnected();
    // Validate the specified port
    if (port > 0xFFFF)
    {
        STHROWF("Port number is out of range: %u > %u", port, 0xFFFF);
    }
    // Validate the specified server
    else if (!server || strlen(server) <= 0)
    {
        STHROWF("Attempting to connect IRC without specifying a server");
    }
    // Validate the specified nickname
    else if (!nick || strlen(nick) <= 0)
    {
        STHROWF("Attempting to connect IRC without specifying a nickname");
    }
    // Save the specified port
    m_Port = port;
    // Save the specified server
    m_Server.assign(server);
    // Save the specified nickname
    m_Nick.assign(nick);
    // Save the specified password
    m_Passwd.assign(passwd ? passwd : _SC(""));
    // Save the specified user
    m_User.assign(user ? user : _SC(""));
    // Save the specified name
    m_Name.assign(name ? name : _SC(""));
    // Enable the reconnection system
    m_Reconnect = true;
    // Reset the number of tries
    m_LeftTries = m_Tries;
    // Set the time-point for the next connection try
    m_NextTry = (_SqMod->GetEpochTimeMicro() + (m_Wait * 1000LL));
    // This is an IPv6 connection
    m_IPv6 = true;
    // Attempt to connect the session and return the result
    return irc_connect6(m_Session, m_Server.c_str(), m_Port,
                        m_Passwd.empty() ? nullptr : m_Passwd.c_str(),
                        m_Nick.c_str(),
                        m_User.empty() ? nullptr : m_User.c_str(),
                        m_Name.empty() ? nullptr : m_Name.c_str()
    );
}

// ------------------------------------------------------------------------------------------------
void Session::Disconnect()
{
    if (Connected())
    {
        // Update one last time to catch remaining events
        Update();
        // Disable the reconnection system
        m_Reconnect = false;
        // Attempt to disconnect
        irc_disconnect(m_Session);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::ForwardEvent(Function & listener, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    // Is there anyone even listening to this event?
    if (listener.IsNull())
    {
        return; // No point in going forward
    }
    // Make sure that the origin can't be a null pointer
    else if (!origin)
    {
        origin = _SC("");
    }
    // Each event must have an array of parameters (empty or not)
    Array parameters(_SqVM, count);
    // Are the any parameters?
    if (params && count > 0)
    {
        // Transform the parameters into a squirrel array
        for (Uint32 i = 0; i < count; ++i)
        {
            parameters.SetValue(i, params[i]);
        }
    }
    // Attempt top execute the callback with the obtained values
    try
    {
        listener.Execute< CSStr, CSStr, Array & >(event, origin, parameters);
    }
    catch (const Sqrat::Exception & e)
    {
        _SqMod->LogErr("IRC event [%s] => Squirrel error [%s]", event, e.Message().c_str());
    }
    catch (const std::exception & e)
    {
        _SqMod->LogErr("IRC event [%s] => Program error [%s]", event, e.what());
    }
    catch (...)
    {
        _SqMod->LogErr("IRC event [%s] => Unknown error", event);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::ForwardEvent(Function & listener, Uint32 event,
                            CCStr origin, CCStr * params, Uint32 count)
{
    // Is there anyone even listening to this event?
    if (listener.IsNull())
    {
        return; // No point in going forward
    }
    // Make sure that the origin can't be a null pointer
    else if (!origin)
    {
        origin = _SC("");
    }
    // Each event must have an array of parameters (empty or not)
    Array parameters(_SqVM, count);
    // Are the any parameters?
    if (params && count > 0)
    {
        // Transform the parameters into a squirrel array
        for (unsigned int i = 0; i < count; ++i)
        {
            parameters.SetValue(i, params[i]);
        }
    }
    // Attempt top execute the callback with the obtained values
    try
    {
        listener.Execute< Uint32, CSStr, Array & >(event, origin, parameters);
    }
    catch (const Sqrat::Exception & e)
    {
        _SqMod->LogErr("IRC event [%s] => Squirrel error [%s]", event, e.Message().c_str());
    }
    catch (const std::exception & e)
    {
        _SqMod->LogErr("IRC event [%s] => Program error [%s]", event, e.what());
    }
    catch (...)
    {
        _SqMod->LogErr("IRC event [%s] => Unknown error", event);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::ForwardEvent(Function & /*listener*/, CCStr /*nick*/,
                            CCStr /*addr*/, irc_dcc_t /*dccid*/)
{
    /* TODO! */
}

// ------------------------------------------------------------------------------------------------
void Session::ForwardEvent(Function & /*listener*/, CCStr /*nick*/,
                            CCStr /*addr*/, CCStr /*filename*/, Ulong /*size*/, irc_dcc_t /*dccid*/)
{
    /* TODO! */
}

// ------------------------------------------------------------------------------------------------
void Session::OnConnect(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        // Prevent any attempts to reconnect now
        inst->m_Reconnect = false;
        // Save the connection time-stamp to calculate session uptime
        inst->m_SessionTime = _SqMod->GetEpochTimeMicro();
        // Now forward event
        ForwardEvent(inst->m_OnConnect, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnNick(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        if (params && params[0])
        {
            inst->m_Nick.assign(params[0]);
        }
        ForwardEvent(inst->m_OnNick, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnQuit(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnQuit, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnJoin(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnJoin, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnPart(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnPart, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnMode(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnMode, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnUmode(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnUmode, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnTopic(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnTopic, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnKick(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnKick, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnChannel(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnChannel, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnPrivMsg(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnPrivMsg, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnNotice(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnNotice, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnChannelNotice(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
       ForwardEvent(inst->m_OnChannelNotice, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnInvite(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnInvite, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnCtcpReq(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnCtcpReq, event, origin, params, count);
    }

}

// ------------------------------------------------------------------------------------------------
void Session::OnCtcpRep(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnCtcpRep, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnCtcpAction(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnCtcpAction, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnUnknown(irc_session_t * session, CCStr event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnUnknown, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnNumeric(irc_session_t * session, Uint32 event, CCStr origin, CCStr * params, Uint32 count)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnNumeric, event, origin, params, count);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnDccChatReq(irc_session_t * session, CCStr nick, CCStr addr, irc_dcc_t dccid)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnDccChatReq, nick, addr, dccid);
    }
}

// ------------------------------------------------------------------------------------------------
void Session::OnDccSendReq(irc_session_t * session, CCStr nick, CCStr addr, CCStr filename, Ulong size, irc_dcc_t dccid)
{
    Session * inst = reinterpret_cast< Session * >(irc_get_ctx(session));
    if (ValidateEventSession(inst))
    {
        ForwardEvent(inst->m_OnDccSendReq, nick, addr, filename, size, dccid);
    }
}

// ------------------------------------------------------------------------------------------------
SQInteger Session::CmdMsgF(HSQUIRRELVM vm)
{
    // Obtain the initial stack size
    const Int32 top = sq_gettop(vm);
    // Do we have a target value?
    if (top <= 1)
    {
        return sq_throwerror(vm, "Missing the message target");
    }
    // Do we have a message value?
    else if (top <= 2)
    {
        return sq_throwerror(vm, "Missing the message value");
    }
    // The session instance
    Session * session = nullptr;
    // Attempt to extract the argument values
    try
    {
        session = Var< Session * >(vm, 1).value;
    }
    catch (const Sqrat::Exception & e)
    {
        // Propagate the error
        return sq_throwerror(vm, e.Message().c_str());
    }
    // Do we have a valid session instance?
    if (!session)
    {
        return sq_throwerror(vm, "Invalid session instance");
    }
    // Do we have a valid session?
    else if (!session->m_Session)
    {
        return sq_throwerror(vm, "Invalid IRC session");
    }
    // Is the session connected?
    else if (!session->Connected())
    {
        return sq_throwerror(vm, "Session is not connected");
    }
    // Attempt to retrieve the target from the stack as a string
    StackStrF target(vm, 2, false);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(target.mRes))
    {
        return target.mRes; // Propagate the error!
    }
    // Attempt to retrieve the value from the stack as a string
    StackStrF message(vm, 3);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(message.mRes))
    {
        return message.mRes; // Propagate the error!
    }
    // Forward the resulted string value and save the returned result code
    const Int32 code = irc_cmd_msg(session->m_Session, target.mPtr, message.mPtr);
    // Push the obtained code onto the stack
    sq_pushinteger(vm, code);
    // We have a value on the stack
    return 1;
}

// ------------------------------------------------------------------------------------------------
SQInteger Session::CmdMeF(HSQUIRRELVM vm)
{
    // Obtain the initial stack size
    const Int32 top = sq_gettop(vm);
    // Do we have a target value?
    if (top <= 1)
    {
        return sq_throwerror(vm, "Missing the message target");
    }
    // Do we have a message value?
    else if (top <= 2)
    {
        return sq_throwerror(vm, "Missing the message value");
    }
    // The session instance
    Session * session = nullptr;
    // Attempt to extract the argument values
    try
    {
        session = Var< Session * >(vm, 1).value;
    }
    catch (const Sqrat::Exception & e)
    {
        // Propagate the error
        return sq_throwerror(vm, e.Message().c_str());
    }
    // Do we have a valid session instance?
    if (!session)
    {
        return sq_throwerror(vm, "Invalid session instance");
    }
    // Do we have a valid session?
    else if (!session->m_Session)
    {
        return sq_throwerror(vm, "Invalid IRC session");
    }
    // Is the session connected?
    else if (!session->Connected())
    {
        return sq_throwerror(vm, "Session is not connected");
    }
    // Attempt to retrieve the target from the stack as a string
    StackStrF target(vm, 2, false);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(target.mRes))
    {
        return target.mRes; // Propagate the error!
    }
    // Attempt to retrieve the value from the stack as a string
    StackStrF message(vm, 3);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(message.mRes))
    {
        return message.mRes; // Propagate the error!
    }
    // Forward the resulted string value and save the returned result code
    const Int32 code = irc_cmd_me(session->m_Session, target.mPtr, message.mPtr);
    // Push the obtained code onto the stack
    sq_pushinteger(vm, code);
    // We have a value on the stack
    return 1;
}

// ------------------------------------------------------------------------------------------------
SQInteger Session::CmdNoticeF(HSQUIRRELVM vm)
{
    // Obtain the initial stack size
    const Int32 top = sq_gettop(vm);
    // Do we have a target value?
    if (top <= 1)
    {
        return sq_throwerror(vm, "Missing the message target");
    }
    // Do we have a message value?
    else if (top <= 2)
    {
        return sq_throwerror(vm, "Missing the message value");
    }
    // The session instance
    Session * session = nullptr;
    // Attempt to extract the argument values
    try
    {
        session = Var< Session * >(vm, 1).value;
    }
    catch (const Sqrat::Exception & e)
    {
        // Propagate the error
        return sq_throwerror(vm, e.Message().c_str());
    }
    // Do we have a valid session instance?
    if (!session)
    {
        return sq_throwerror(vm, "Invalid session instance");
    }
    // Do we have a valid session?
    else if (!session->m_Session)
    {
        return sq_throwerror(vm, "Invalid IRC session");
    }
    // Is the session connected?
    else if (!session->Connected())
    {
        return sq_throwerror(vm, "Session is not connected");
    }
    // Attempt to retrieve the target from the stack as a string
    StackStrF target(vm, 2, false);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(target.mRes))
    {
        return target.mRes; // Propagate the error!
    }
    // Attempt to retrieve the value from the stack as a string
    StackStrF message(vm, 3);
    // Have we failed to retrieve the string?
    if (SQ_FAILED(message.mRes))
    {
        return message.mRes; // Propagate the error!
    }
    // Forward the resulted string value and save the returned result code
    const Int32 code = irc_cmd_notice(session->m_Session, target.mPtr, message.mPtr);
    // Push the obtained code onto the stack
    sq_pushinteger(vm, code);
    // We have a value on the stack
    return 1;
}

} // Namespace:: SqMod
