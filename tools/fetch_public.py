"""Fetch public project sources with the existing local proxy, without credentials."""
import argparse
from pathlib import Path
import urllib.request

def fetch(url, output):
    opener = urllib.request.build_opener(urllib.request.ProxyHandler({
        'https': 'http://127.0.0.1:7892', 'http': 'http://127.0.0.1:7892'}))
    request = urllib.request.Request(url, headers={'User-Agent': 'cie-vdot-research', 'server': '1'})
    target = Path(output)
    target.parent.mkdir(parents=True, exist_ok=True)
    with opener.open(request, timeout=60) as response, target.open('wb') as stream:
        while chunk := response.read(1024 * 1024):
            stream.write(chunk)
    print(f'{target}: {target.stat().st_size} bytes', flush=True)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('url')
    parser.add_argument('output')
    args = parser.parse_args()
    fetch(args.url, args.output)
