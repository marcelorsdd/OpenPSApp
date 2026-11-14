import { existsSync, readdirSync, readFileSync } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import type { Plugin } from 'vite';

export interface ExtraNotice {
  name: string;
  license: string;
  file?: string;
  text?: string;
}

interface Section {
  title: string;
  text: string;
}

const LICENSE_FILE = /^(licen[cs]e|copying)(\.(md|txt))?$/i;
const MARKER = '/node_modules/';
const RULE = '='.repeat(72);
const APACHE_2 = fileURLToPath(new URL('./licenses/apache-2.0.txt', import.meta.url));

export function packageFile(from: string, name: string, file: string): string {
  for (let dir = from; ; dir = path.dirname(dir)) {
    const candidate = path.join(dir, 'node_modules', name, file);
    if (existsSync(candidate)) return candidate;
    if (path.dirname(dir) === dir) throw new Error(`${name}/${file} not found in node_modules`);
  }
}

export function apache2(copyright: string): string {
  return `${copyright}\n\n${readFileSync(APACHE_2, 'utf8').trim()}`;
}

export function mit(copyright: string): string {
  return `MIT License

${copyright}

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.`;
}

function packageRoot(id: string): string | null {
  const at = id.lastIndexOf(MARKER);
  if (at < 0) return null;
  const parts = id.slice(at + MARKER.length).split('/');
  const name = parts[0].startsWith('@') ? `${parts[0]}/${parts[1]}` : parts[0];
  return id.slice(0, at + MARKER.length) + name;
}

function licenseFile(dir: string): string | null {
  const file = readdirSync(dir).find((f) => LICENSE_FILE.test(f));
  return file ? readFileSync(path.join(dir, file), 'utf8').trim() : null;
}

function packageSection(root: string, unbundled: Record<string, string>): Section {
  const pkg = JSON.parse(readFileSync(path.join(root, 'package.json'), 'utf8'));
  const text = licenseFile(root) ?? unbundled[pkg.name];
  if (!text) throw new Error(`no license file in ${root}`);
  return { title: `${pkg.name} ${pkg.version} (${pkg.license})`, text };
}

function extraSection(extra: ExtraNotice): Section {
  const text = extra.file ? readFileSync(extra.file, 'utf8').trim() : (extra.text ?? '');
  return { title: `${extra.name} (${extra.license})`, text };
}

export default function licenses(header: string, extras: ExtraNotice[], unbundled: Record<string, string> = {}): Plugin {
  return {
    name: 'third-party-licenses',
    apply: 'build',
    generateBundle(_, bundle) {
      const roots = new Set<string>();
      for (const output of Object.values(bundle)) {
        if (output.type !== 'chunk') continue;
        for (const id of output.moduleIds) {
          const root = packageRoot(id.replace(/\?.*$/, ''));
          if (root) roots.add(root);
        }
      }
      const sections = [...[...roots].map((root) => packageSection(root, unbundled)), ...extras.map(extraSection)].sort((a, b) =>
        a.title.localeCompare(b.title),
      );
      const body = [header, ...sections.map((s) => `${RULE}\n${s.title}\n${RULE}\n\n${s.text}`)].join('\n\n');
      this.emitFile({ type: 'asset', fileName: 'licenses.txt', source: `${body}\n` });
    },
  };
}
