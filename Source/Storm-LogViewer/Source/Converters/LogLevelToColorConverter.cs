using Storm_LogViewer.Source.Log;
using System;
using System.Globalization;
using System.Windows.Data;
using System.Windows.Media;

namespace Storm_LogViewer.Source.Converters
{
    class LogLevelToColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            LogLevelEnum logLevel = (LogLevelEnum)value;

            switch (logLevel)
            {
            case Log.LogLevelEnum.Debug: return new SolidColorBrush(Color.FromRgb(0, 200, 50));
            case Log.LogLevelEnum.DebugError: return new SolidColorBrush(Color.FromRgb(180, 25, 25));
            case Log.LogLevelEnum.Comment: return new SolidColorBrush(Color.FromRgb(150, 150, 150));
            case Log.LogLevelEnum.Warning: return new SolidColorBrush(Color.FromRgb(200, 200, 0));
            case Log.LogLevelEnum.Error: return new SolidColorBrush(Color.FromRgb(220, 90, 50));
            case Log.LogLevelEnum.Fatal: return new SolidColorBrush(Color.FromRgb(225, 5, 5));
            case Log.LogLevelEnum.Always: return new SolidColorBrush(Color.FromRgb(200, 200, 200));
            case Log.LogLevelEnum.None: return new SolidColorBrush(Color.FromRgb(0, 0, 0));
            default: return new SolidColorBrush(Color.FromRgb(255, 255, 255));
            }
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
