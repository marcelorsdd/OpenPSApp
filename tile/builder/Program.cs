using System.Text.Json;
using LibProsperoPkg;

if (args.Length != 2)
{
    Console.Error.WriteLine("usage: PlayTrackTile <source-dir> <output-dir>");
    return 2;
}

string source = Path.GetFullPath(args[0]);
string output = Path.GetFullPath(args[1]);
using var param = JsonDocument.Parse(File.ReadAllText(Path.Combine(source, "sce_sys", "param.json")));
var root = param.RootElement;

var options = new ProsperoBuildOptions
{
    Mode = ProsperoPackageMode.Application,
    OutputFormat = ProsperoOutputFormat.DebugImage,
    SourceFolder = source,
    OutputFolder = output,
    ContentId = root.GetProperty("contentId").GetString()!,
    TitleId = root.GetProperty("titleId").GetString()!,
    Title = root.GetProperty("localizedParameters").GetProperty("en-US").GetProperty("titleName").GetString()!,
    Version = root.GetProperty("masterVersion").GetString()!,
    GenerateParamJsonIfMissing = false,
    LicenseFree = true,
};

Directory.CreateDirectory(output);
var result = ProsperoPackageBuilder.Build(options, Console.WriteLine);
Console.WriteLine(JsonSerializer.Serialize(result, new JsonSerializerOptions { WriteIndented = true }));
return 0;
