import { readdirSync, readFileSync } from 'node:fs';
import path from 'node:path';
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

function packageRoot(id: string): string | null {
  const at = id.lastIndexOf(MARKER);
  if (at < 0) return null;
  const parts = id.slice(at + MARKER.length).split('/');
  const name = parts[0].startsWith('@') ? `${parts[0]}/${parts[1]}` : parts[0];
  return id.slice(0, at + MARKER.length) + name;
}

function licenseText(dir: string): string {
  const file = readdirSync(dir).find((f) => LICENSE_FILE.test(f));
  if (!file) throw new Error(`no license file in ${dir}`);
  return readFileSync(path.join(dir, file), 'utf8').trim();
}

function packageSection(root: string): Section {
  const pkg = JSON.parse(readFileSync(path.join(root, 'package.json'), 'utf8'));
  return { title: `${pkg.name} ${pkg.version} (${pkg.license})`, text: licenseText(root) };
}

function extraSection(extra: ExtraNotice): Section {
  const text = extra.file ? readFileSync(extra.file, 'utf8').trim() : (extra.text ?? '');
  return { title: `${extra.name} (${extra.license})`, text };
}

export default function licenses(header: string, extras: ExtraNotice[]): Plugin {
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
      const sections = [...[...roots].map(packageSection), ...extras.map(extraSection)].sort((a, b) =>
        a.title.localeCompare(b.title),
      );
      const body = [header, ...sections.map((s) => `${RULE}\n${s.title}\n${RULE}\n\n${s.text}`)].join('\n\n');
      this.emitFile({ type: 'asset', fileName: 'licenses.txt', source: `${body}\n` });
    },
  };
}
