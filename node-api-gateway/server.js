
import 'dotenv/config'
import express from 'express'

const app = express()
const PORT = process.env.PORT



app.use(express.json());

app.listen(PORT, () => {
  console.log('Server is running on http://localhost:3000')
})



