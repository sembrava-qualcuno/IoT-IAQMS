FROM node:18-alpine

# Create app directory
WORKDIR /usr/src/app

# Install app dependencies
COPY package*.json ./

RUN npm install

# Bundle app source
COPY index.js ./

EXPOSE 3000
CMD ["node", "index.js"]
