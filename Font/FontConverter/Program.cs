using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Firefly;
using Firefly.Imaging;

namespace FontConverter
{
    internal class Program
    {
        static void Main(string[] args)
        {
            var Command = args[0];
            var InputPath = args[1];
            var OutputPath = args[2];
            if (Command == "bitmap")
            {
                using (var i = Bmp.Open(InputPath))
                using (var o = new Bmp(OutputPath + ".bmp", i.Width, i.Height, 1))
                {
                    var r = i.GetRectangle(0, 0, i.Width, i.Height);
                    o.Palette = new Int32[] { unchecked((int)(0xFF000000)), unchecked((int)(0xFFFFFFFF)) };
                    for (int y = 0; y < i.Height; y += 1)
                    {
                        for (int x = 0; x < i.Width; x += 1)
                        {
                            r[x, y] = r[x, y] == 0 ? 0 : 1;
                        }
                    }
                    o.SetRectangle(0, 0, r);

                    var Lines = new List<String>();
                    var LineBytes = new List<Byte>();
                    for (int y = 0; y < i.Height; y += 1)
                    {
                        for (int x = 0; x < i.Width; x += 8)
                        {
                            Byte b = 0;
                            for (int k = 0; (k < 8) && (x + k < i.Width); k += 1)
                            {
                                b = (Byte)(b | (r[x + k, y] << k));
                            }
                            LineBytes.Add(b);
                            if (LineBytes.Count >= 16)
                            {
                                Lines.Add(String.Join(", ", LineBytes.Select(bb => "0x" + bb.ToString("X2"))));
                                LineBytes.Clear();
                            }
                        }
                    }
                    if (LineBytes.Count > 0)
                    {
                        Lines.Add(String.Join(", ", LineBytes.Select(bb => "0x" + bb.ToString("X2"))));
                        LineBytes.Clear();
                    }
                    var Text = $"unsigned char font_bitmap[{i.Width * i.Height / 8}] =\n{{\n{String.Join(",\n", Lines)}\n}};\n";
                    System.IO.File.WriteAllText(OutputPath, Text);
                }
            }
            else if (Command == "desc")
            {
                var Glyphs = Firefly.Glyphing.FdGlyphDescriptionFile.ReadFile(InputPath).OrderBy(g => g.c.Unicodes).ToList();
                var Lines = Glyphs.Select(g => $"{{0x{g.c.Unicodes.Single().Value.ToString("X4")}, {g.PhysicalBox.X}, {g.PhysicalBox.Y}, {g.PhysicalBox.Width}, {g.PhysicalBox.Height}, {g.VirtualBox.X}, {g.VirtualBox.Y}, {g.VirtualBox.Width}, {g.VirtualBox.Height}}}").ToList();
                var Text = $"const GlyphDescription font_bitmap_desc[{Glyphs.Count}] =\n{{\n{String.Join(",\n", Lines)}\n}};\n";
                System.IO.File.WriteAllText(OutputPath, Text);
            }
        }
    }
}
