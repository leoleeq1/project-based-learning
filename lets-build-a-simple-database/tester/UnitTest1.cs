using System.Diagnostics;

namespace tester;

public class DBTester
{
    private static Process? RunProcess()
    {
        return Process.Start(new ProcessStartInfo
        {
            FileName = Path.GetFullPath("d:/gitproject/project-based-learning/build/main.exe"),
            RedirectStandardInput = true,
            RedirectStandardOutput = true,
        });
    }

    private static void WriteLines(StreamWriter sw, IEnumerable<string> lines)
    {
        foreach (var line in lines)
        {
            sw.WriteLine(line);
        }
    }

    private static void ReadLines(StreamReader sr, IEnumerable<string> lines)
    {
        foreach (var line in lines)
        {
            var read = sr.ReadLine();
            Assert.Equal(line, read);
        }
    }

    [Fact]
    public void InsertsAndRetrievesARow()
    {
        using var process = RunProcess();
        Assert.NotNull(process);

        WriteLines(process.StandardInput, [
            "insert 1 user1 person1@example.com",
            "select",
            ".exit",
        ]);

        ReadLines(process.StandardOutput, [
            "db > Executed.",
            "db > (1, user1, person1@example.com)",
            "Executed.",
            "db > ",
        ]);
    }

    [Fact]
    public void PrintsErrorMessageWhenTableIsFull()
    {
        using var process = RunProcess();
        Assert.NotNull(process);

        var list = new List<string>();
        process.OutputDataReceived += (s, e) =>
        {
            list.Add(e.Data ?? string.Empty);
        };
        process.BeginOutputReadLine();

        for (var i = 0; i < 1401; ++i)
        {
            process.StandardInput.WriteLine($"insert {i} person{i} person{i}@example.com");
        }
        process.StandardInput.WriteLine(".exit");
        process.StandardInput.Flush();

        process.WaitForExit();

        Assert.Equal("db > Error: Table full.", list[^3]);
    }

    [Fact]
    public void AllowsInsertingStringsThatAreTheMaximumLength()
    {
        var longUserName = new string('a', 32);
        var longEmail = new string('a', 255);

        using var process = RunProcess();
        Assert.NotNull(process);

        WriteLines(process.StandardInput, [
            $"insert 1 {longUserName} {longEmail}",
            "select",
            ".exit",
        ]);

        ReadLines(process.StandardOutput, [
            "db > Executed.",
            $"db > (1, {longUserName}, {longEmail})",
            "Executed.",
            "db > ",
        ]);
    }

    [Fact]
    public void PrintsErrorMessageIfStringsAreTooLong()
    {
        var longUserName = new string('a', 33);
        var longEmail = new string('a', 256);

        using var process = RunProcess();
        Assert.NotNull(process);

        WriteLines(process.StandardInput, [
            $"insert 1 {longUserName} {longEmail}",
            "select",
            ".exit",
        ]);

        ReadLines(process.StandardOutput, [
            "db > String is too long.",
            "db > Executed.",
            "db > ",
        ]);
    }

    [Fact]
    public void PrintsAnErrorMessageIfIdIsNegative()
    {
        using var process = RunProcess();
        Assert.NotNull(process);

        WriteLines(process.StandardInput, [
            $"insert -1 cstack foo@bar.com",
            "select",
            ".exit",
        ]);

        ReadLines(process.StandardOutput, [
            "db > ID must be positive.",
            "db > Executed.",
            "db > ",
        ]);
    }
}
