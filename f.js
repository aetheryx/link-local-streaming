const Jimp = require('jimp');
const fs = require('node:fs');

const raw = fs.readFileSync('/Users/zain/Development/zainclient/out.bin');

const image = new Jimp(3456, 1440);
for (let i = 0; i < raw.length; i += 4) {
  image.bitmap.data[i] = raw[i + 2];
  image.bitmap.data[i + 1] = raw[i + 1];
  image.bitmap.data[i + 2] = raw[i];
  image.bitmap.data[i + 3] = raw[i + 3];
}

image.write('/Users/zain/Development/zainclient/desktop.png', () => {
  console.log('done');
});
