using Storm_LogViewer.Source.Log;
using System;
using System.Globalization;
using System.Windows.Data;

namespace Storm_LogViewer.Source.Converters
{
    class LogLevelToStringConverter : IValueConverter
    {
        public static LogLevelEnum FromString(string value)
        {
            LogLevelEnum level;
            if (Enum.TryParse<LogLevelEnum>(value, true, out level))
            {
                return level;
            }

            return LogLevelEnum.Unknown;
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return ((LogLevelEnum)value).ToString();
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return LogLevelToStringConverter.FromString(value as string);
        }
    }
}
