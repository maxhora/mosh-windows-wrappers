using System;
using System.Configuration;
using System.Diagnostics;
using System.IO;
using System.Net;
using System.Reflection;

namespace mosh
{
    internal static class MoshClientWrapper
    {
        internal const string MoshClientAppSettingsKey = "mosh-client";
        private const string MoshClientExeName = "mosh-client.exe";

        private static readonly Lazy<string> MoshClientExePathLazy = new Lazy<string>(() => {
            AppSettingsReader asr = new AppSettingsReader();

            string path;

            try
            {
                path = (string)asr.GetValue(MoshClientAppSettingsKey, typeof(string));

                if (!string.IsNullOrEmpty(path) && File.Exists(path))
                    return path;
            }
            catch
            {
                // ignored
            }

            if (File.Exists(MoshClientExeName))
                return MoshClientExeName;

            path = new FileInfo(Assembly.GetExecutingAssembly().Location).Directory?.FullName;

            if (string.IsNullOrEmpty(path))
                return null;

            path = Path.Combine(path, MoshClientExeName);

            return File.Exists(path) ? path : null;
        });

        internal static string MoshClientExePath => MoshClientExePathLazy.Value;

        internal static int StartMoshSession(string moshUser, IPAddress host, string moshPort, string moshKey)
        {
            using (Process sshProcess = new Process())
            {
                sshProcess.StartInfo.FileName = MoshClientExePath;
                sshProcess.StartInfo.UseShellExecute = false;
                sshProcess.StartInfo.RedirectStandardInput = false;
                sshProcess.StartInfo.RedirectStandardOutput = false;
                sshProcess.StartInfo.RedirectStandardError = false;
                sshProcess.StartInfo.Arguments = $"{host} {moshPort}";
                sshProcess.StartInfo.EnvironmentVariables.Add("MOSH_KEY", moshKey);
                sshProcess.StartInfo.EnvironmentVariables.Add("MOSH_USER", moshUser);

                sshProcess.Start();

                sshProcess.WaitForExit();

                return sshProcess.ExitCode;
            }
        }

    }
}