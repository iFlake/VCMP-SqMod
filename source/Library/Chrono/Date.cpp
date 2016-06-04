// ------------------------------------------------------------------------------------------------
#include "Library/Chrono/Date.hpp"
#include "Library/Chrono/Date.hpp"
#include "Library/Chrono/Datetime.hpp"
#include "Base/Shared.hpp"

// ------------------------------------------------------------------------------------------------
namespace SqMod {

// ------------------------------------------------------------------------------------------------
SQChar Date::Delimiter = '-';

// ------------------------------------------------------------------------------------------------
const Uint8 Date::MonthLengths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

// ------------------------------------------------------------------------------------------------
SQInteger Date::Typename(HSQUIRRELVM vm)
{
    static const SQChar name[] = _SC("SqDate");
    sq_pushstring(vm, name, sizeof(name));
    return 1;
}

// ------------------------------------------------------------------------------------------------
Date Date::operator + (const Date & o) const
{
    // Add the components individually
    return Date(o);
}

// ------------------------------------------------------------------------------------------------
Date Date::operator - (const Date & o) const
{
    return Date(o);
}

// ------------------------------------------------------------------------------------------------
Date Date::operator * (const Date & o) const
{
    return Date(o);
}

// ------------------------------------------------------------------------------------------------
Date Date::operator / (const Date & o) const
{
    return Date(o);
}

// ------------------------------------------------------------------------------------------------
void Date::Set(Uint16 year, Uint8 month, Uint8 day)
{
    if (!ValidDate(year, month, day))
    {
        STHROWF("Invalid date: %04u%c%02u%c%02u%c%u"
            , m_Delimiter, m_Year
            , m_Delimiter, m_Month
            , m_Delimiter, m_Day
        );
    }
}

// ------------------------------------------------------------------------------------------------
CSStr Date::GetStr() const
{
    return ToString();
}

// ------------------------------------------------------------------------------------------------
void Date::SetStr(CSStr str)
{
    // The format specifications that will be used to scan the string
    static SQChar fs[] = _SC(" %u , %u , %u,");
    // Is the specified string empty?
    if (!str || *str == '\0')
    {
        // Clear the values
        m_Year = 0;
        m_Month = 0;
        m_Day = 0;
        // We're done here
        return;
    }
    // Assign the specified delimiter
    fs[4] = m_Delimiter;
    fs[9] = m_Delimiter;
    // The sscanf function requires at least 32 bit integers
    Uint32 year = 0, month = 0, day = 0;
    // Attempt to extract the component values from the specified string
    sscanf(str, fs, &year, &month, &day);
    // Clamp the extracted values to the boundaries of associated type and assign them
    Set(ClampL< Uint32, Uint8 >(year),
        ClampL< Uint32, Uint8 >(month),
        ClampL< Uint32, Uint8 >(day)
    );
}

// ------------------------------------------------------------------------------------------------
Int32 Date::Compare(const Date & o) const
{
    if (m_Year < o.m_Year)
    {
        return -1;
    }
    else if (m_Year > o.m_Year)
    {
        return 1;
    }
    else if (m_Month < o.m_Month)
    {
        return -1;
    }
    else if (m_Month > o.m_Month)
    {
        return 1;
    }
    else if (m_Day < o.m_Day)
    {
        return -1;
    }
    else if (m_Day > o.m_Day)
    {
        return 1;
    }
    // They're equal
    return 0;
}

// ------------------------------------------------------------------------------------------------
CSStr Date::ToString() const
{
    return ToStrF("%04u%c%02u%c%02u%c%u"
        , m_Delimiter, m_Year
        , m_Delimiter, m_Month
        , m_Delimiter, m_Day
    );
}

// ------------------------------------------------------------------------------------------------
void Date::SetDayOfYear(Uint16 doy)
{
    // Reverse the given day of year to a full date
    Date d = ReverseDayOfyear(m_Year, doy);
    // Set the obtained month
    SetMonth(d.m_Month);
    // Set the obtained day
    SetDay(d.m_Day);
}

// ------------------------------------------------------------------------------------------------
void Date::SetYear(Uint16 year)
{
    // Make sure the year is valid
    if (!year)
    {
        STHROWF("Invalid year: %u", year);
    }
    // Assign the value
    m_Year = year;
    // Make sure the new date is valid
    if (!ValidDate(m_Year, m_Month, m_Day))
    {
        m_Month = 1;
        m_Day = 1;
    }
}

// ------------------------------------------------------------------------------------------------
void Date::SetMonth(Uint8 month)
{
    // Make sure the month is valid
    if (month == 0 || month > 12)
    {
        STHROWF("Invalid month: %u", month);
    }
    // Assign the value
    m_Month = month;
    // Make sure the month days are in range
    if (m_Day > DaysInMonth(m_Year, m_Month))
    {
        m_Month = 1; // Fall back to the beginning of the month
    }
}

// ------------------------------------------------------------------------------------------------
void Date::SetDay(Uint8 day)
{
    // Grab the amount of days in the current month
    const Uint8 dim = DaysInMonth(m_Year, m_Month);
    // Make sure the day is valid
    if (day == 0)
    {
        STHROWF("Invalid day: %u", day);
    }
    else if (day > dim)
    {
        STHROWF("Day is out of range: %u > %u", day, dim);
    }
    // Assign the value
    m_Day = day;
}

// ------------------------------------------------------------------------------------------------
void Date::AddYears(Int32 years)
{
    // Do we have a valid amount of years?
    if (years)
    {
        // Add the specified amount of years
        SetYear(ConvTo< Uint16 >::From(static_cast< Int32 >(m_Year) + years));
    }
}

// ------------------------------------------------------------------------------------------------
void Date::AddMonths(Int32 months)
{
    // Do we have a valid amount of months?
    if (!months)
    {
        // Calculate the the years, if any
        Int32 years = static_cast< Int32 >(months / 12);
        // Calculate the months, if any
        months = (months % 12) + m_Month;
        // Do we have extra months?
        if (months >= 12)
        {
            ++years;
            months %= 12;
        }
        else if (months < 0)
        {
            --years;
            months = 12 - months;
        }
        // Are there any years to add?
        if (years)
        {
            SetYear(ConvTo< Uint16 >::From(static_cast< Int32 >(m_Year) + years));
        }
        // Add the months
        SetMonth(months);
    }
}

// ------------------------------------------------------------------------------------------------
void Date::AddDays(Int32 days)
{
    // Do we have a valid amount of days?
    if (!days)
    {
        // Should we go in a positive or negative direction?
        Int32 dir = days > 0 ? 1 : -1;
        // Grab current year
        Int32 year = m_Year;
        // Calculate the days in the current year
        Int32 diy = DaysInYear(year);
        // Calculate the day of year
        Int32 doy = GetDayOfYear() + days;
        // Calculate the resulting years
        while (doy > diy || doy < 0)
        {
            doy -= diy * dir;
            year += dir;
            diy = DaysInYear(year);
        }
        // Set the obtained year
        SetYear(year);
        // Set the obtained day of year
        SetDayOfYear(doy);
    }
}

// ------------------------------------------------------------------------------------------------
Date Date::AndYears(Int32 years)
{
    // Do we have a valid amount of years?
    if (!years)
    {
        return Date(*this);
    }
    // Replicate the current date
    Date d(*this);
    // Add the specified amount of years
    d.SetYear(ConvTo< Uint16 >::From(static_cast< Int32 >(m_Year) + years));
    // Return the resulted date
    return d;
}

// ------------------------------------------------------------------------------------------------
Date Date::AndMonths(Int32 months)
{
    // Do we have a valid amount of months?
    if (!months)
    {
        return Date(*this);
    }
    // Calculate the the years, if any
    Int32 years = static_cast< Int32 >(months / 12);
    // Calculate the months, if any
    months = (months % 12) + m_Month;
    // Do we have extra months?
    if (months >= 12)
    {
        ++years;
        months %= 12;
    }
    else if (months < 0)
    {
        --years;
        months = 12 - months;
    }
    // Replicate the current date
    Date d(*this);
    // Are there any years to add?
    if (years)
    {
        d.SetYear(ConvTo< Uint16 >::From(static_cast< Int32 >(m_Year) + years));
    }
    // Add the months
    d.SetMonth(months);
    // Return the resulted date
    return d;
}

// ------------------------------------------------------------------------------------------------
Date Date::AndDays(Int32 days)
{
    // Do we have a valid amount of days?
    if (!days)
    {
        return Date(*this);
    }
    // Should we go in a positive or negative direction?
    Int32 dir = days > 0 ? 1 : -1;
    // Grab current year
    Int32 year = m_Year;
    // Calculate the days in the current year
    Int32 diy = DaysInYear(year);
    // Calculate the day of year
    Int32 doy = GetDayOfYear() + days;
    // Calculate the resulting years
    while (doy > diy || doy < 0)
    {
        doy -= diy * dir;
        year += dir;
        diy = DaysInYear(year);
    }
    // Replicate the current date
    Date d(*this);
    // Set the obtained year
    d.SetYear(year);
    // Set the obtained day of year
    d.SetDayOfYear(doy);
    // Return the resulted date
    return d;
}

// ------------------------------------------------------------------------------------------------
bool Date::ValidDate(Uint16 year, Uint8 month, Uint8 day)
{
    // Is this a valid date?
    if (year == 0 || month == 0 || day == 0)
    {
        return false;
    }
    // Is the month within range?
    else if (month > 12)
    {
        return false;
    }
    // Return whether the day inside the month
    return day <= DaysInMonth(year, month);
}

// ------------------------------------------------------------------------------------------------
Uint8 Date::DaysInMonth(Uint16 year, Uint8 month)
{
    // Is the specified month within range?
    if (month > 12)
    {
        STHROWF("Month value is out of range: %u > 12", month);
    }
    // Obtain the days in this month
    Uint8 days = *(MonthLengths + month);
    // Should we account for January?
    if (month == 2 && IsLeapYear(year))
    {
        ++days;
    }
    // Return the resulted days
    return days;
}

// ------------------------------------------------------------------------------------------------
Uint16 Date::DayOfYear(Uint16 year, Uint8 month, Uint8 day)
{
    // Start with 0 days
    Uint16 doy = 0;
    // Cumulate the days in months
    for (Uint8 m = 1; m < month; ++month)
    {
        doy += DaysInMonth(year, m);
    }
    // Add the specified days
    doy += day;
    // Return the result
    return doy;
}

// ------------------------------------------------------------------------------------------------
Date Date::ReverseDayOfyear(Uint16 year, Uint16 doy)
{
    // The resulted month
    Uint8 month = 1;
    // Calculate the months till the specified day of year
    for (; month < 12; ++month)
    {
        // Get the number of days in the current month
        Uint32 days = DaysInMonth(year, month);
        // Can this month fit in the remaining days?
        if (days >= doy)
        {
            break; // The search is complete
        }
        // Subtract the month days from days of year
        doy -= days;
    }
    // Return the resulted date
    return Date(year, month, doy);
}

// ================================================================================================
void Register_ChronoDate(HSQUIRRELVM vm, Table & /*cns*/)
{
    RootTable(vm).Bind(_SC("SqDate"), Class< Date >(vm, _SC("SqDate"))
        // Constructors
        .Ctor()
        .Ctor< Uint16 >()
        .Ctor< Uint16, Uint8 >()
        .Ctor< Uint16, Uint8, Uint8 >()
        // Static Properties
        .SetStaticValue(_SC("GlobalDelimiter"), &Date::Delimiter)
        // Core Meta-methods
        .Func(_SC("_tostring"), &Date::ToString)
        .SquirrelFunc(_SC("_typename"), &Date::Typename)
        .Func(_SC("_cmp"), &Date::Cmp)
        // Meta-methods
        .Func< Date (Date::*)(const Date &) const >(_SC("_add"), &Date::operator +)
        .Func< Date (Date::*)(const Date &) const >(_SC("_sub"), &Date::operator -)
        .Func< Date (Date::*)(const Date &) const >(_SC("_mul"), &Date::operator *)
        .Func< Date (Date::*)(const Date &) const >(_SC("_div"), &Date::operator /)
        // Properties
        .Prop(_SC("Delimiter"), &Date::GetDelimiter, &Date::SetDelimiter)
        .Prop(_SC("DayOfYear"), &Date::GetDayOfYear, &Date::SetDayOfYear)
        .Prop(_SC("Str"), &Date::GetStr, &Date::SetStr)
        .Prop(_SC("Year"), &Date::GetYear, &Date::SetYear)
        .Prop(_SC("Month"), &Date::GetMonth, &Date::SetMonth)
        .Prop(_SC("Day"), &Date::GetDay, &Date::SetDay)
        .Prop(_SC("LeapYear"), &Date::IsThisLeapYear)
        .Prop(_SC("YearDays"), &Date::GetYearDays)
        .Prop(_SC("MonthDays"), &Date::GetMonthDays)
        // Member Methods
        .Func(_SC("AddYears"), &Date::AddYears)
        .Func(_SC("AddMonths"), &Date::AddMonths)
        .Func(_SC("AddDays"), &Date::AddDays)
        .Func(_SC("AndYears"), &Date::AndYears)
        .Func(_SC("AndMonths"), &Date::AndMonths)
        .Func(_SC("AndDays"), &Date::AndDays)
        // Overloaded Methods
        .Overload< void (Date::*)(Uint16) >(_SC("Set"), &Date::Set)
        .Overload< void (Date::*)(Uint16, Uint8) >(_SC("Set"), &Date::Set)
        .Overload< void (Date::*)(Uint16, Uint8, Uint8) >(_SC("Set"), &Date::Set)
        // Static Functions
        .StaticFunc(_SC("IsLeapYear"), &Date::IsLeapYear)
        .StaticFunc(_SC("IsValidDate"), &Date::ValidDate)
        .StaticFunc(_SC("GetDaysInYear"), &Date::DaysInYear)
        .StaticFunc(_SC("GetDaysInMonth"), &Date::DaysInMonth)
        .StaticFunc(_SC("GetDayOfYear"), &Date::DayOfYear)
        .StaticFunc(_SC("GetReverseDayOfyear"), &Date::ReverseDayOfyear)
    );
}

} // Namespace:: SqMod
