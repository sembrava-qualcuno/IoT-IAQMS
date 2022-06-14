FROM node:18-alpine

# Create app directory
WORKDIR /usr/src/app

# Install app dependencies
COPY package*.json ./

RUN npm install

# Bundle app source
COPY index.js ./
COPY public/ ./public/
COPY tsconfig.json ./

EXPOSE 3002
CMD ["node", "index.js"]
